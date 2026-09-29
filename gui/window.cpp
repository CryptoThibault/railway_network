#include "window.hpp"
#include "simulation_process.hpp"
#include <X11/Xft/Xft.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <algorithm>
#include <array>
#include <cerrno>
#include <fcntl.h>
#include <iomanip>
#include <poll.h>
#include <sstream>
#include <stdexcept>
#include <string>

static std::string decimal(double value)
{
    std::ostringstream text;
    text << std::fixed << std::setprecision(1) << value;
    return text.str();
}

struct ApplicationWindow::Implementation
{
    enum Shade { Background, Panel, Border, Text, Muted, Mint, Hover, Blue, Amber, Track, Count };
    Display* display = nullptr;
    ::Window handle = 0;
    GC graphics = nullptr;
    Pixmap buffer = 0;
    XftDraw* canvas = nullptr;
    std::array<XftFont*, 4> fonts{};
    std::array<XftColor, Count> colors{};
    int allocatedColors = 0;
    Atom closeMessage = 0;
    Atom protocols = 0;
    int width = 1024;
    int height = 720;
    int bufferWidth = 0;
    int bufferHeight = 0;
    bool hovered = false;
    bool pressed = false;
    bool pauseHovered = false;
    bool pausePressed = false;
    int speedPressed = 0;
    int speedHovered = 0;
    SimulationProcess simulation{NETWORK_PROJECT_ROOT};

    ~Implementation()
    {
        if (!display)
            return;
        if (canvas)
            XftDrawDestroy(canvas);
        if (buffer)
            XFreePixmap(display, buffer);
        for (auto* font : fonts)
            if (font)
                XftFontClose(display, font);
        for (int i = 0; i < allocatedColors; ++i)
            XftColorFree(display, DefaultVisual(display, DefaultScreen(display)),
                DefaultColormap(display, DefaultScreen(display)), &colors[i]);
        if (graphics)
            XFreeGC(display, graphics);
        if (handle)
            XDestroyWindow(display, handle);
        XCloseDisplay(display);
    }

    void initializeGraphics()
    {
        const int screen = DefaultScreen(display);
        const std::array<const char*, Count> names{
            "#0B1220", "#141F30", "#253348", "#F1F5FA", "#94A6BE",
            "#58DFC0", "#8CEDD6", "#79B7FF", "#F3BE70", "#2B3C52"};
        for (const char* name : names)
        {
            if (!XftColorAllocName(display, DefaultVisual(display, screen),
                    DefaultColormap(display, screen), name, &colors[allocatedColors]))
                throw std::runtime_error("Cannot allocate dashboard colors");
            ++allocatedColors;
        }
        const std::array<const char*, 4> descriptions{
            "sans:size=10", "sans:size=14", "sans:style=Bold:size=22", "sans:style=Bold:size=48"};
        for (std::size_t i = 0; i < fonts.size(); ++i)
        {
            fonts[i] = XftFontOpenName(display, screen, descriptions[i]);
            if (!fonts[i])
                throw std::runtime_error("Cannot load dashboard fonts");
        }
    }

    bool insideButton(int x, int y) const
    {
        return x >= width - 180 && x < width - 36 && y >= 38 && y < 86;
    }

    bool insidePause(int x, int y) const
    {
        return x >= width - 340 && x < width - 196 && y >= 38 && y < 86;
    }

    int speedAt(int x, int y) const
    {
        const int column = (width - 104) / 3;
        const int left = 36 + 2 * (column + 16) + 22;
        const int buttonWidth = (column - 44) / 5;
        if (buttonWidth <= 0 || y < 250 || y >= 282 || x < left || x >= left + buttonWidth * 5)
            return 0;
        return ((x - left) / buttonWidth + 1) * 60;
    }

    void rectangle(int x, int y, int w, int h, Shade shade)
    {
        XSetForeground(display, graphics, colors[shade].pixel);
        XFillRectangle(display, buffer, graphics, x, y, std::max(1, w), std::max(1, h));
    }

    void rounded(int x, int y, int w, int h, int radius, Shade shade)
    {
        rectangle(x + radius, y, w - radius * 2, h, shade);
        rectangle(x, y + radius, w, h - radius * 2, shade);
        XSetForeground(display, graphics, colors[shade].pixel);
        for (int dx : {0, w - radius * 2})
            for (int dy : {0, h - radius * 2})
                XFillArc(display, buffer, graphics, x + dx, y + dy,
                    radius * 2, radius * 2, 0, 360 * 64);
    }

    void card(int x, int y, int w, int h)
    {
        rounded(x, y, w, h, 16, Border);
        rounded(x + 1, y + 1, w - 2, h - 2, 15, Panel);
    }

    int textWidth(const std::string& value, int font)
    {
        XGlyphInfo extents;
        XftTextExtentsUtf8(display, fonts[font],
            reinterpret_cast<const FcChar8*>(value.data()), value.size(), &extents);
        return extents.xOff;
    }

    void text(std::string value, int x, int y, int font, Shade shade, int maxWidth = 0)
    {
        if (maxWidth > 0 && textWidth(value, font) > maxWidth)
        {
            while (!value.empty() && textWidth(value + "...", font) > maxWidth)
            {
                std::size_t last = value.size() - 1;
                while (last > 0 && (static_cast<unsigned char>(value[last]) & 0xc0) == 0x80)
                    --last;
                value.erase(last);
            }
            value += "...";
        }
        XftDrawStringUtf8(canvas, &colors[shade], fonts[font], x, y,
            reinterpret_cast<const FcChar8*>(value.data()), value.size());
    }

    void drawTrain(int center, int railY)
    {
        const int x = center - 30;
        rounded(x, railY - 29, 60, 24, 8, Mint);
        rectangle(x + 5, railY - 12, 49, 5, Blue);
        rounded(x + 8, railY - 25, 11, 9, 2, Background);
        rounded(x + 24, railY - 25, 11, 9, 2, Background);
        rounded(x + 43, railY - 25, 10, 9, 2, Background);
        rectangle(x + 57, railY - 15, 3, 4, Text);
        for (int wheel : {13, 46})
        {
            XSetForeground(display, graphics, colors[Background].pixel);
            XFillArc(display, buffer, graphics, x + wheel - 5, railY - 10, 10, 10, 0, 360 * 64);
            XSetForeground(display, graphics, colors[Muted].pixel);
            XFillArc(display, buffer, graphics, x + wheel - 2, railY - 7, 4, 4, 0, 360 * 64);
        }
    }

    void draw()
    {
        if (width <= 0 || height <= 0)
            return;
        if (!buffer || width != bufferWidth || height != bufferHeight)
        {
            if (canvas)
                XftDrawDestroy(canvas);
            canvas = nullptr;
            if (buffer)
                XFreePixmap(display, buffer);
            buffer = XCreatePixmap(display, handle, width, height,
                DefaultDepth(display, DefaultScreen(display)));
            canvas = XftDrawCreate(display, buffer, DefaultVisual(display, DefaultScreen(display)),
                DefaultColormap(display, DefaultScreen(display)));
            if (!canvas)
                throw std::runtime_error("Cannot create dashboard canvas");
            bufferWidth = width;
            bufferHeight = height;
        }
        rectangle(0, 0, width, height, Background);
        const auto& telemetry = simulation.getTelemetry();
        text("Railway Network", 36, 62, 2, Text);
        text("LIVE TRAIN MONITOR", 38, 93, 0, Muted);
        rounded(width - 180, 38, 144, 48, 12, hovered || pressed ? Hover : Mint);
        const std::string button = simulation.isRunning() ? "Stop" : "Start";
        text(button, width - 108 - textWidth(button, 1) / 2, 69, 1, Background);

        const bool canPause = simulation.isRunning() && telemetry.has_value();
        rounded(width - 340, 38, 144, 48, 12,
            canPause && (pauseHovered || pausePressed) ? Border : Panel);
        const std::string pauseLabel = simulation.isPaused() ? "Resume" : "Pause";
        text(pauseLabel, width - 268 - textWidth(pauseLabel, 1) / 2, 69, 1,
            canPause ? (simulation.isPaused() ? Amber : Text) : Muted);

        const int gap = 16;
        const int column = (width - 72 - gap * 2) / 3;
        const int stateX = 36 + column + gap;
        const int clockX = stateX + column + gap;
        card(36, 130, column, 192);
        card(stateX, 130, column, 192);
        card(clockX, 130, width - 36 - clockX, 192);
        text("SPEED", 58, 163, 0, Muted);
        text(telemetry ? decimal(telemetry->speed) : "--", 56, 242,
            column < 250 ? 2 : 3, Mint, column - 40);
        text("km/h", 58, 290, 1, Muted);

        const std::string state = telemetry ? telemetry->state : "Ready";
        const Shade stateShade = state == "Braking" ? Amber : (state == "Cruising" ? Blue : Mint);
        text("TRAIN STATE", stateX + 22, 163, 0, Muted);
        text(state, stateX + 22, 227, 1, stateShade, column - 44);
        text(telemetry ? telemetry->type + " / Train " + telemetry->train : "Awaiting departure",
            stateX + 22, 290, 0, Muted, column - 44);

        text("SIMULATED TIME", clockX + 22, 163, 0, Muted);
        text(telemetry ? telemetry->time : "0h0m0s", clockX + 22, 227, 1, Text, column - 44);
        const int buttonWidth = (column - 44) / 5;
        for (int hours = 1; hours <= 5; ++hours)
        {
            const int x = clockX + 22 + (hours - 1) * buttonWidth;
            const bool selected = simulation.getTimeScale() == hours * 60;
            rounded(x, 250, buttonWidth - 3, 32, 6,
                selected ? Mint : (speedHovered == hours * 60 ? Border : Track));
            const std::string label = std::to_string(hours) + "h";
            text(label, x + (buttonWidth - 3 - textWidth(label, 0)) / 2, 272,
                0, selected ? Background : Text);
        }
        text("1 min = " + std::to_string(simulation.getTimeScale() / 60) + "h / "
            + std::to_string(simulation.getTimeScale()) + "x", clockX + 22, 307, 0,
            simulation.isPaused() ? Amber : Blue, column - 44);

        const int routeHeight = std::max(230, height - 450);
        card(36, 342, width - 72, routeHeight);
        text("CURRENT SEGMENT", 58, 375, 0, Muted);
        const std::string departure = telemetry ? telemetry->departure : "Departure";
        const std::string destination = telemetry ? telemetry->destination : "Destination";
        text(departure, 58, 421, 2, Text, (width - 140) / 2);
        text(destination, width / 2 + 20, 421, 2, Text, (width - 140) / 2);
        text("FROM", 58, 447, 0, Muted);
        text("TO", width / 2 + 20, 447, 0, Muted);

        const int trackWidth = width - 140;
        const double ratio = telemetry ? std::clamp(telemetry->position / telemetry->length, 0.0, 1.0) : 0.0;
        for (int sleeper = 70; sleeper <= width - 70; sleeper += 18)
            rectangle(sleeper, 481, 4, 13, Border);
        rectangle(70, 483, trackWidth, 2, Track);
        rectangle(70, 490, trackWidth, 2, Track);
        const int progress = static_cast<int>(trackWidth * ratio);
        if (progress > 0)
            rectangle(70, 490, progress, 2, Mint);
        drawTrain(70 + progress, 483);
        text(telemetry ? decimal(telemetry->position) + " / " + decimal(telemetry->length) + " km" : "-- / -- km",
            58, 529, 1, Text);
        const std::string percent = decimal(ratio * 100.0) + "%";
        text(percent, width - 58 - textWidth(percent, 1), 529, 1, Mint);
        const std::string legs = "Completed legs: " + (telemetry ? std::to_string(telemetry->legs) : "0");
        text(legs, width - 58 - textWidth(legs, 0), 375, 0, Muted);

        const int footer = 342 + routeHeight;
        text(simulation.getStatus(), 38, footer + 36, 0, simulation.isPaused() ? Amber : (simulation.isRunning() ? Mint : Muted), width - 76);
        text("Enter: start / stop     Space: pause / resume     1-5: speed     Esc: close", 38, footer + 65, 0, Muted);
        XCopyArea(display, buffer, handle, graphics, 0, 0, width, height, 0, 0);
        XFlush(display);
    }

    void start()
    {
        if (simulation.isRunning())
            simulation.stop();
        else
            simulation.start();
    }
};

ApplicationWindow::ApplicationWindow() : implementation(std::make_unique<Implementation>())
{
    auto& state = *implementation;
    state.display = XOpenDisplay(nullptr);
    if (!state.display)
        throw std::runtime_error("Cannot open display. Run network_window in an X11 or XWayland desktop session.");
    const int connection = ConnectionNumber(state.display);
    const int flags = fcntl(connection, F_GETFD);
    if (flags < 0 || fcntl(connection, F_SETFD, flags | FD_CLOEXEC) < 0)
        throw std::runtime_error("Cannot configure display connection");
    state.initializeGraphics();
    const int screen = DefaultScreen(state.display);
    state.handle = XCreateSimpleWindow(state.display, RootWindow(state.display, screen),
        0, 0, state.width, state.height, 0, state.colors[Implementation::Background].pixel,
        state.colors[Implementation::Background].pixel);
    state.graphics = XCreateGC(state.display, state.handle, 0, nullptr);
    if (!state.graphics)
        throw std::runtime_error("Cannot initialize dashboard graphics");
    XSetGraphicsExposures(state.display, state.graphics, False);
    XStoreName(state.display, state.handle, "Railway Network");
    XSizeHints hints{};
    hints.flags = PMinSize;
    hints.min_width = 760;
    hints.min_height = 660;
    XSetWMNormalHints(state.display, state.handle, &hints);
    XSelectInput(state.display, state.handle, StructureNotifyMask | ExposureMask
        | KeyPressMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask
        | LeaveWindowMask | FocusChangeMask);
    state.closeMessage = XInternAtom(state.display, "WM_DELETE_WINDOW", False);
    state.protocols = XInternAtom(state.display, "WM_PROTOCOLS", False);
    XSetWMProtocols(state.display, state.handle, &state.closeMessage, 1);
    XMapWindow(state.display, state.handle);
    XFlush(state.display);
}

ApplicationWindow::~ApplicationWindow() = default;

void ApplicationWindow::run()
{
    auto& state = *implementation;
    while (true)
    {
        if (state.simulation.update())
            state.draw();
        if (XPending(state.display) == 0)
        {
            pollfd connection{ConnectionNumber(state.display), POLLIN, 0};
            const int result = poll(&connection, 1, 50);
            if (result < 0 && errno != EINTR)
                throw std::runtime_error("Cannot wait for window events");
            if (connection.revents & (POLLERR | POLLHUP | POLLNVAL))
                throw std::runtime_error("Display connection closed");
            continue;
        }
        XEvent event;
        XNextEvent(state.display, &event);
        if (event.type == ClientMessage && event.xclient.message_type == state.protocols
            && event.xclient.format == 32
            && static_cast<Atom>(event.xclient.data.l[0]) == state.closeMessage)
            return;
        if (event.type == DestroyNotify && event.xdestroywindow.window == state.handle)
        {
            state.handle = 0;
            return;
        }
        if (event.type == KeyPress)
        {
            const KeySym key = XLookupKeysym(&event.xkey, 0);
            if (key == XK_Escape)
                return;
            if (key == XK_Return || key == XK_KP_Enter)
                state.start();
            else if (key == XK_space)
                state.simulation.togglePause();
            else if (key >= XK_1 && key <= XK_5)
                state.simulation.setTimeScale(static_cast<int>(key - XK_0) * 60);
            else if (key >= XK_KP_1 && key <= XK_KP_5)
                state.simulation.setTimeScale(static_cast<int>(key - XK_KP_0) * 60);
        }
        else if (event.type == ConfigureNotify)
        {
            state.width = event.xconfigure.width;
            state.height = event.xconfigure.height;
        }
        else if (event.type == MotionNotify)
        {
            state.hovered = state.insideButton(event.xmotion.x, event.xmotion.y);
            state.pauseHovered = state.insidePause(event.xmotion.x, event.xmotion.y);
            state.speedHovered = state.speedAt(event.xmotion.x, event.xmotion.y);
        }
        else if (event.type == LeaveNotify || event.type == FocusOut)
        {
            state.hovered = false;
            state.pressed = false;
            state.pauseHovered = false;
            state.speedHovered = 0;
            state.speedPressed = 0;
            state.pausePressed = false;
        }
        else if (event.type == ButtonPress && event.xbutton.button == Button1)
        {
            state.pressed = state.insideButton(event.xbutton.x, event.xbutton.y);
            state.pausePressed = state.insidePause(event.xbutton.x, event.xbutton.y);
            state.speedPressed = state.speedAt(event.xbutton.x, event.xbutton.y);
        }
        else if (event.type == ButtonRelease && event.xbutton.button == Button1)
        {
            if (state.pressed && state.insideButton(event.xbutton.x, event.xbutton.y))
                state.start();
            if (state.pausePressed && state.insidePause(event.xbutton.x, event.xbutton.y))
                state.simulation.togglePause();
            if (state.speedPressed != 0 && state.speedPressed == state.speedAt(event.xbutton.x, event.xbutton.y))
                state.simulation.setTimeScale(state.speedPressed);
            state.speedPressed = 0;
            state.pausePressed = false;
            state.pressed = false;
        }
        state.draw();
    }
}
