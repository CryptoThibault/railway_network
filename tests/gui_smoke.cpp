#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>

static void require(bool condition, const std::string& message)
{
    if (!condition)
        throw std::runtime_error(message);
}

class WindowFixture
{
public:
    explicit WindowFixture(const char* executable)
    {
        display = XOpenDisplay(nullptr);
        if (!display)
            throw std::runtime_error("Cannot open X11 display for GUI check");
        process = fork();
        if (process == 0)
        {
            execl(executable, executable, static_cast<char*>(nullptr));
            _exit(127);
        }
        require(process > 0, "Cannot launch GUI check");
    }

    ~WindowFixture()
    {
        if (process > 0)
        {
            kill(process, SIGTERM);
            waitpid(process, nullptr, 0);
        }
        if (display)
            XCloseDisplay(display);
    }

    Window find(Window root)
    {
        Atom actual;
        int format;
        unsigned long count;
        unsigned long remaining;
        unsigned char* bytes = nullptr;
        const Atom pidAtom = XInternAtom(display, "_NET_WM_PID", False);
        if (XGetWindowProperty(display, root, pidAtom, 0, 1, False, XA_CARDINAL,
            &actual, &format, &count, &remaining, &bytes) == Success)
        {
            const bool ours = actual == XA_CARDINAL && format == 32 && count == 1 && bytes
                && *reinterpret_cast<unsigned long*>(bytes) == static_cast<unsigned long>(process);
            if (bytes)
                XFree(bytes);
            if (ours)
                return root;
        }
        Window parent;
        Window ancestor;
        Window* children = nullptr;
        unsigned int childCount = 0;
        if (!XQueryTree(display, root, &ancestor, &parent, &children, &childCount))
            return 0;
        Window found = 0;
        for (unsigned int index = 0; index < childCount && !found; ++index)
            found = find(children[index]);
        if (children)
            XFree(children);
        return found;
    }

    void waitForWindow()
    {
        for (int attempt = 0; attempt < 100; ++attempt)
        {
            window = find(DefaultRootWindow(display));
            if (window)
            {
                XWindowAttributes attributes;
                XGetWindowAttributes(display, window, &attributes);
                if (attributes.map_state == IsViewable)
                    return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        throw std::runtime_error("GUI window did not appear");
    }

    void key(KeySym symbol)
    {
        XEvent event{};
        event.xkey.type = KeyPress;
        event.xkey.display = display;
        event.xkey.window = window;
        event.xkey.root = DefaultRootWindow(display);
        event.xkey.same_screen = True;
        event.xkey.keycode = XKeysymToKeycode(display, symbol);
        XSendEvent(display, window, False, KeyPressMask, &event);
        XFlush(display);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void resize(int width, int height)
    {
        XResizeWindow(display, window, width, height);
        XFlush(display);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::vector<unsigned char> capture(const std::filesystem::path& path)
    {
        XWindowAttributes attributes;
        XGetWindowAttributes(display, window, &attributes);
        XImage* frame = XGetImage(display, window, 0, 0, attributes.width, attributes.height, AllPlanes, ZPixmap);
        require(frame != nullptr, "Cannot capture GUI preview");
        std::vector<unsigned char> pixels;
        pixels.reserve(attributes.width * attributes.height * 3);
        for (int y = 0; y < attributes.height; ++y)
            for (int x = 0; x < attributes.width; ++x)
            {
                const auto pixel = XGetPixel(frame, x, y);
                for (auto mask : {frame->red_mask, frame->green_mask, frame->blue_mask})
                {
                    unsigned int shift = 0;
                    while (((mask >> shift) & 1) == 0)
                        ++shift;
                    pixels.push_back(static_cast<unsigned char>(((pixel & mask) >> shift) * 255 / (mask >> shift)));
                }
            }
        XDestroyImage(frame);
        std::ofstream output(path, std::ios::binary);
        output << "P6\n" << attributes.width << ' ' << attributes.height << "\n255\n";
        output.write(reinterpret_cast<const char*>(pixels.data()), pixels.size());
        return pixels;
    }

    void close()
    {
        XEvent event{};
        event.xclient.type = ClientMessage;
        event.xclient.window = window;
        event.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", False);
        event.xclient.format = 32;
        event.xclient.data.l[0] = static_cast<long>(XInternAtom(display, "WM_DELETE_WINDOW", False));
        XSendEvent(display, window, False, NoEventMask, &event);
        XFlush(display);
        for (int attempt = 0; attempt < 100; ++attempt)
        {
            int status;
            if (waitpid(process, &status, WNOHANG) == process)
            {
                process = -1;
                require(WIFEXITED(status) && WEXITSTATUS(status) == 0, "GUI did not close cleanly");
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        throw std::runtime_error("GUI did not close");
    }

    Display* display = nullptr;
    Window window = 0;
    pid_t process = -1;
};

int main(int argc, char** argv)
{
    try
    {
        require(argc == 3, "Expected GUI executable and preview directory");
        const std::filesystem::path directory = argv[2];
        std::filesystem::create_directories(directory);
        WindowFixture fixture(argv[1]);
        fixture.waitForWindow();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        fixture.capture(directory / "ready.ppm");
        fixture.key(XK_space);
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        fixture.key(XK_space);
        const auto paused = fixture.capture(directory / "one-train.ppm");
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
        require(paused == fixture.capture(directory / "paused.ppm"), "Paused dashboard must remain visually frozen");
        fixture.key(XK_5);
        fixture.key(XK_space);
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        fixture.key(XK_space);
        fixture.capture(directory / "multiple-trains.ppm");
        fixture.resize(1000, 680);
        const auto compact = fixture.capture(directory / "compact.ppm");
        fixture.key(XK_End);
        require(compact != fixture.capture(directory / "scrolled.ppm"), "Train list did not scroll");
        fixture.key(XK_Home);
        require(compact == fixture.capture(directory / "home.ppm"), "Scrolling Home did not restore the top of the list");
        fixture.resize(1480, 1000);
        fixture.capture(directory / "wide.ppm");
        fixture.key(XK_3);
        fixture.key(XK_space);
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        fixture.key(XK_space);
        fixture.capture(directory / "resumed.ppm");
        fixture.close();
        std::cout << "GUI smoke checks passed; previews in " << directory << '\n';
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
