#include "simulation_process.hpp"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <chrono>
#include <csignal>
#include <thread>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>

SimulationProcess::SimulationProcess(std::filesystem::path directory)
    : directory(std::move(directory))
{
}

SimulationProcess::~SimulationProcess()
{
    stop();
    if (capture >= 0)
        close(capture);
}

bool SimulationProcess::readOutput()
{
    if (capture < 0)
        return false;
    bool changed = false;
    char buffer[8192];
    for (int batch = 0; batch < 32; ++batch)
    {
        const ssize_t count = read(capture, buffer, sizeof(buffer));
        if (count < 0 && errno == EINTR)
            continue;
        if (count == 0)
        {
            close(capture);
            capture = -1;
            break;
        }
        if (count < 0)
            break;
        telemetry.append(std::string_view(buffer, static_cast<std::size_t>(count)));
        output.append(buffer, static_cast<std::size_t>(count));
        constexpr std::size_t limit = 1024 * 1024;
        if (output.size() > limit)
            output.erase(0, output.size() - limit);
        changed = true;
    }
    return changed;
}

void SimulationProcess::start()
{
    if (isRunning())
        return;

    paused = false;
    telemetry.reset();
    output.clear();
    if (capture >= 0)
        close(capture);
    capture = -1;
    const auto executable = directory / "network";
    if (access(executable.c_str(), X_OK) != 0)
    {
        status = "Cannot launch network: " + std::string(std::strerror(errno));
        return;
    }

    int descriptors[2];
    if (pipe2(descriptors, O_CLOEXEC) < 0)
    {
        status = "Cannot capture simulation output: " + std::string(std::strerror(errno));
        return;
    }
    capture = descriptors[0];
    if (fcntl(capture, F_SETFL, O_NONBLOCK) < 0)
    {
        close(capture);
        close(descriptors[1]);
        capture = -1;
        status = "Cannot configure simulation output";
        return;
    }

    const std::string scaleArgument = std::to_string(timeScale);
    process = fork();
    if (process == 0)
    {
        if (dup2(descriptors[1], STDOUT_FILENO) < 0
            || dup2(descriptors[1], STDERR_FILENO) < 0)
            _exit(126);
        if (chdir(directory.c_str()) != 0)
            _exit(126);
        execl(executable.c_str(), executable.c_str(), "--speed", scaleArgument.c_str(), static_cast<char*>(nullptr));
        _exit(127);
    }
    close(descriptors[1]);
    if (process < 0)
        status = "Cannot start simulation: " + std::string(std::strerror(errno));
    else
        status = "Simulation running...";
}

bool SimulationProcess::update()
{
    const bool changed = readOutput();
    if (!isRunning())
        return changed;
    int result = 0;
    const pid_t finished = waitpid(process, &result, WNOHANG);
    if (finished == 0 || (finished < 0 && errno == EINTR))
        return changed;
    readOutput();
    process = -1;
    paused = false;
    if (finished < 0)
        status = "Cannot read simulation result";
    else if (WIFEXITED(result) && WEXITSTATUS(result) == 0)
        status = "Simulation complete";
    else if (WIFEXITED(result))
        status = "Simulation failed (exit " + std::to_string(WEXITSTATUS(result)) + ")";
    else
        status = "Simulation interrupted";
    return true;
}

bool SimulationProcess::isRunning() const
{
    return process > 0;
}

const std::string& SimulationProcess::getStatus() const
{
    return status;
}

const std::string& SimulationProcess::getOutput() const
{
    return output;
}

void SimulationProcess::stop()
{
    if (!isRunning())
        return;
    kill(process, SIGTERM);
    for (int attempt = 0; attempt < 100 && isRunning(); ++attempt)
    {
        update();
        if (isRunning())
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    if (isRunning())
    {
        kill(process, SIGKILL);
        while (waitpid(process, nullptr, 0) < 0 && errno == EINTR)
        {
        }
        process = -1;
        readOutput();
    }
    paused = false;
    status = "Simulation stopped";
}

const std::optional<TrainTelemetry>& SimulationProcess::getTelemetry() const
{
    return telemetry.getSnapshot();
}

void SimulationProcess::togglePause()
{
    if (!isRunning() || !telemetry.getSnapshot())
        return;
    if (kill(process, paused ? SIGUSR2 : SIGUSR1) != 0)
    {
        status = "Cannot change pause state: " + std::string(std::strerror(errno));
        return;
    }
    paused = !paused;
    status = paused ? "Simulation paused" : "Simulation running...";
}

bool SimulationProcess::isPaused() const
{
    return paused;
}

void SimulationProcess::setTimeScale(int scale)
{
    if (scale < 60 || scale > 300)
        return;
    if (isRunning())
    {
        if (!telemetry.getSnapshot())
            return;
        union sigval value{};
        value.sival_int = scale;
        if (sigqueue(process, SIGRTMIN, value) != 0)
        {
            status = "Cannot change simulation speed: " + std::string(std::strerror(errno));
            return;
        }
    }
    timeScale = scale;
}

int SimulationProcess::getTimeScale() const
{
    return timeScale;
}

bool SimulationProcess::needsUpdate() const
{
    return isRunning() || capture >= 0;
}
