#include "window.hpp"
#include "simulation_controller.hpp"
#include <X11/Xft/Xft.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <poll.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>

static std::string number(double value, int precision = 1)
{
    std::ostringstream output;
    output << std::fixed << std::setprecision(precision) << value;
    return output.str();
}

static std::string clockTime(double time)
{
    const auto seconds = static_cast<unsigned long long>(time + 1e-6);
    std::ostringstream output;
    output << std::setfill('0') << std::setw(2) << seconds / 3600 % 24
        << ':' << std::setw(2) << seconds / 60 % 60 << ':' << std::setw(2) << seconds % 60;
    return output.str();
}

static std::string duration(double time)
{
    const auto seconds = static_cast<unsigned long>(std::max(0.0, std::ceil(time)));
    std::ostringstream output;
    output << seconds / 60 << ':' << std::setfill('0') << std::setw(2) << seconds % 60;
    return output.str();
}

struct ApplicationWindow::Implementation
{
    enum Shade { Background, Surface, Border, Ink, Muted, Teal, TealLight, Blue, BlueLight,
        Amber, AmberLight, Track, White, Count };
    Display* display = nullptr;
    ::Window handle = 0;
    GC graphics = nullptr;
    Pixmap buffer = 0;
    XftDraw* canvas = nullptr;
    std::array<XftFont*, 5> fonts{};
    std::array<XftColor, Count> colors{};
    int allocatedColors = 0;
    Atom closeMessage = 0;
    Atom protocols = 0;
    int width = 1240;
    int height = 900;
    int bufferWidth = 0;
    int bufferHeight = 0;
    int scroll = 0;
    int hovered = 0;
    int pressed = 0;
    static constexpr int viewportTop = 181;
    static constexpr int cardHeight = 312;
    static constexpr int cardGap = 18;
    static constexpr std::array<int, 5> speeds{60, 300, 1200, 2400, 6000};
    SimulationController simulation{std::filesystem::path(NETWORK_PROJECT_ROOT) / "data.json",
        std::filesystem::path(NETWORK_PROJECT_ROOT) / "log"};

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
        for (int index = 0; index < allocatedColors; ++index)
            XftColorFree(display, DefaultVisual(display, DefaultScreen(display)),
                DefaultColormap(display, DefaultScreen(display)), &colors[index]);
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
            "#F3F5F7", "#FFFFFF", "#E2E7EC", "#172C3B", "#70808D", "#147D73",
            "#E6F3EF", "#3568AB", "#EAF0FA", "#A76C16", "#FFF4DF", "#CBD5DD", "#FFFFFF"};
        for (const auto* name : names)
        {
            if (!XftColorAllocName(display, DefaultVisual(display, screen),
                DefaultColormap(display, screen), name, &colors[allocatedColors]))
                throw std::runtime_error("Cannot allocate dashboard colors");
            ++allocatedColors;
        }
        const std::array<const char*, 5> descriptions{
            "DejaVu Sans:size=9", "DejaVu Sans:size=11", "DejaVu Sans:style=Bold:size=14",
            "DejaVu Sans:style=Bold:size=20", "DejaVu Sans Mono:style=Bold:size=17"};
        for (std::size_t index = 0; index < fonts.size(); ++index)
        {
            fonts[index] = XftFontOpenName(display, screen, descriptions[index]);
            if (!fonts[index])
                throw std::runtime_error("Cannot load dashboard fonts");
        }
    }

    void rectangle(int x, int y, int w, int h, Shade shade)
    {
        if (w <= 0 || h <= 0)
            return;
        XSetForeground(display, graphics, colors[shade].pixel);
        XFillRectangle(display, buffer, graphics, x, y, w, h);
    }

    void rounded(int x, int y, int w, int h, int radius, Shade shade)
    {
        radius = std::min({radius, w / 2, h / 2});
        if (radius <= 0)
        {
            rectangle(x, y, w, h, shade);
            return;
        }
        rectangle(x + radius, y, w - radius * 2, h, shade);
        rectangle(x, y + radius, w, h - radius * 2, shade);
        XSetForeground(display, graphics, colors[shade].pixel);
        for (int dx : {0, w - radius * 2})
            for (int dy : {0, h - radius * 2})
                XFillArc(display, buffer, graphics, x + dx, y + dy, radius * 2, radius * 2, 0, 360 * 64);
    }

    void line(int x1, int y1, int x2, int y2, Shade shade, int thickness = 1)
    {
        XSetForeground(display, graphics, colors[shade].pixel);
        XSetLineAttributes(display, graphics, thickness, LineSolid, CapRound, JoinRound);
        XDrawLine(display, buffer, graphics, x1, y1, x2, y2);
        XSetLineAttributes(display, graphics, 1, LineSolid, CapButt, JoinMiter);
    }

    int textWidth(const std::string& value, int font) const
    {
        XGlyphInfo extents;
        XftTextExtentsUtf8(display, fonts[font], reinterpret_cast<const FcChar8*>(value.data()), value.size(), &extents);
        return extents.xOff;
    }

    void text(std::string value, int x, int y, int font, Shade shade, int maxWidth = 0)
    {
        if (maxWidth > 0 && textWidth(value, font) > maxWidth)
        {
            while (!value.empty() && textWidth(value + "…", font) > maxWidth)
            {
                std::size_t last = value.size() - 1;
                while (last > 0 && (static_cast<unsigned char>(value[last]) & 0xc0) == 0x80)
                    --last;
                value.erase(last);
            }
            value += "…";
        }
        XftDrawStringUtf8(canvas, &colors[shade], fonts[font], x, y,
            reinterpret_cast<const FcChar8*>(value.data()), value.size());
    }

    void centered(const std::string& value, int x, int y, int font, Shade shade, int maxWidth = 0)
    {
        text(value, x - std::min(textWidth(value, font), maxWidth > 0 ? maxWidth : textWidth(value, font)) / 2,
            y, font, shade, maxWidth);
    }

    void circle(int x, int y, int radius, Shade shade)
    {
        XSetForeground(display, graphics, colors[shade].pixel);
        XFillArc(display, buffer, graphics, x - radius, y - radius, radius * 2, radius * 2, 0, 360 * 64);
    }

    void trainIcon(int center, int rail, Shade shade)
    {
        const int x = center - 28;
        rounded(x, rail - 23, 42, 18, 5, shade);
        XPoint nose[]{
            {static_cast<short>(x + 35), static_cast<short>(rail - 23)},
            {static_cast<short>(x + 46), static_cast<short>(rail - 23)},
            {static_cast<short>(x + 57), static_cast<short>(rail - 9)},
            {static_cast<short>(x + 53), static_cast<short>(rail - 5)},
            {static_cast<short>(x + 35), static_cast<short>(rail - 5)}};
        XSetForeground(display, graphics, colors[shade].pixel);
        XFillPolygon(display, buffer, graphics, nose, 5, Convex, CoordModeOrigin);
        for (int offset : {7, 17, 27})
            rounded(x + offset, rail - 19, 6, 6, 1, White);
        line(x + 40, rail - 19, x + 45, rail - 14, White, 3);
        line(x + 16, rail - 25, x + 22, rail - 30, Muted);
        line(x + 22, rail - 30, x + 27, rail - 25, Muted);
        line(x + 18, rail - 31, x + 28, rail - 31, Muted);
        for (int offset : {10, 32, 46})
            circle(x + offset, rail - 4, 3, Ink);
    }

    void stationIcon(int x, int rail, bool passed)
    {
        const Shade color = passed ? Teal : Muted;
        circle(x, rail, 5, passed ? Teal : Track);
        circle(x, rail, 2, White);
        rectangle(x - 9, rail + 11, 18, 12, color);
        XPoint roof[]{
            {static_cast<short>(x - 12), static_cast<short>(rail + 11)},
            {static_cast<short>(x), static_cast<short>(rail + 4)},
            {static_cast<short>(x + 12), static_cast<short>(rail + 11)}};
        XSetForeground(display, graphics, colors[color].pixel);
        XFillPolygon(display, buffer, graphics, roof, 3, Convex, CoordModeOrigin);
        rectangle(x - 2, rail + 16, 4, 7, White);
        rectangle(x - 7, rail + 15, 3, 3, White);
        rectangle(x + 4, rail + 15, 3, 3, White);
    }

    int controlAt(int x, int y) const
    {
        if (y >= 29 && y < 70 && x >= width - 166 && x < width - 26)
            return 1;
        if (y >= 35 && y < 70 && x >= 246 && x < 246 + 5 * 63)
        {
            const int index = (x - 246) / 63;
            if ((x - 246) % 63 < 58)
                return index + 2;
        }
        return 0;
    }

    int viewportHeight() const { return std::max(1, height - viewportTop - 37); }
    int contentHeight() const
    {
        return static_cast<int>(simulation.getTrains().size()) * (cardHeight + cardGap);
    }
    int maximumScroll() const { return std::max(0, contentHeight() - viewportHeight()); }

    void activate(int control)
    {
        if (control == 1)
            simulation.togglePlayback();
        else if (control >= 2 && control < 7)
            simulation.setTimeScale(speeds[control - 2]);
    }

    void drawHeader()
    {
        rectangle(0, 0, width, 96, Surface);
        rectangle(0, 95, width, 1, Border);
        rounded(26, 27, 42, 42, 10, TealLight);
        rounded(36, 34, 22, 24, 6, Teal);
        rectangle(40, 38, 14, 8, White);
        circle(42, 53, 2, White);
        circle(52, 53, 2, White);
        line(38, 63, 42, 59, Teal, 2);
        line(55, 63, 51, 59, Teal, 2);
        text("RAILWAY", 80, 43, 2, Ink);
        text("Live services", 80, 64, 0, Muted);
        text("SIMULATION SPEED", 246, 24, 0, Muted);
        for (std::size_t index = 0; index < speeds.size(); ++index)
        {
            const bool selected = simulation.getTimeScale() == speeds[index];
            const int x = 246 + static_cast<int>(index) * 63;
            rounded(x, 35, 58, 35, 7, selected ? Teal : (hovered == static_cast<int>(index) + 2 ? Border : Background));
            centered(std::to_string(speeds[index]) + "×", x + 29, 57, 0, selected ? White : Ink);
        }
        const int clockX = width - 332;
        text("DAY " + std::to_string(static_cast<unsigned long>(simulation.getTime()) / 86400 + 1), clockX, 31, 0, Muted);
        text(clockTime(simulation.getTime()), clockX, 60, 4, Ink);
        const bool available = simulation.getServiceCount() != 0;
        rounded(width - 166, 29, 140, 41, 8, available ? (simulation.isRunning() ? Ink : Teal) : Track);
        if (hovered == 1 && available)
            rounded(width - 166, 29, 140, 41, 8, simulation.isRunning() ? Blue : Teal);
        const int iconX = width - 143;
        if (simulation.isRunning())
        {
            rectangle(iconX, 42, 4, 15, White);
            rectangle(iconX + 8, 42, 4, 15, White);
        }
        else
        {
            XPoint play[]{
                {static_cast<short>(iconX), 41}, {static_cast<short>(iconX), 58},
                {static_cast<short>(iconX + 13), 50}};
            XSetForeground(display, graphics, colors[White].pixel);
            XFillPolygon(display, buffer, graphics, play, 3, Convex, CoordModeOrigin);
        }
        text(simulation.isRunning() ? "Pause" : (simulation.hasStarted() ? "Resume" : "Start"), width - 116, 55, 1, White);
    }

    void drawSection()
    {
        text("On the network", 30, 134, 3, Ink);
        text("Journeys appear at departure and leave the board on arrival.", 31, 158, 1, Muted, width - 290);
        const auto count = simulation.getTrains().size();
        rounded(width - 165, 112, 134, 35, 17, count ? TealLight : Border);
        circle(width - 145, 129, 4, count ? Teal : Muted);
        text(std::to_string(count) + (count == 1 ? " active train" : " active trains"), width - 133, 135, 1, count ? Teal : Muted);
    }

    void metric(int x, int y, int w, const std::string& label, const std::string& value, const std::string& detail)
    {
        text(label, x, y, 0, Muted, w);
        text(value, x, y + 29, 2, Ink, w);
        text(detail, x, y + 50, 0, Muted, w);
    }

    void drawCard(const TrainDashboard& train, int y)
    {
        const int x = 30;
        const int w = width - 74;
        rounded(x, y + 2, w, cardHeight, 12, Border);
        rounded(x, y, w, cardHeight, 12, Surface);
        rectangle(x + 1, y + 22, 3, cardHeight - 44, train.state == TrainState::Waiting ? Amber : Teal);
        text(train.name, x + 24, y + 32, 2, Ink, w - 250);
        text(train.origin + " → " + train.destination + "   ·   Service " + std::to_string(train.id),
            x + 24, y + 55, 1, Muted, w - 250);
        std::string state;
        Shade badge = BlueLight;
        Shade stateColor = Blue;
        switch (train.state)
        {
            case TrainState::Waiting:
                state = "Station stop · " + duration(train.pauseRemaining);
                badge = AmberLight;
                stateColor = Amber;
                break;
            case TrainState::Accelerating: state = "Accelerating"; break;
            case TrainState::Cruising: state = "Cruising"; badge = TealLight; stateColor = Teal; break;
            case TrainState::Braking: state = "Braking"; badge = AmberLight; stateColor = Amber; break;
            default: state = "Waiting"; break;
        }
        const int badgeWidth = std::max(108, textWidth(state, 0) + 28);
        rounded(x + w - badgeWidth - 24, y + 20, badgeWidth, 28, 14, badge);
        centered(state, x + w - badgeWidth / 2 - 24, y + 39, 0, stateColor);
        const int column = (w - 48) / 4;
        metric(x + 24, y + 85, column - 12, "SPEED", number(train.speed) + " m/s",
            number(train.speed * 3.6, 0) + " km/h · limit " + number(train.segmentLimit) + " m/s");
        metric(x + 24 + column, y + 85, column - 12,
            train.station.empty() ? "CURRENT LEG" : "AT THE STATION",
            train.station.empty() ? train.legOrigin + " → " + train.legDestination : train.station,
            train.station.empty() ? "Moving towards " + train.legDestination : "Continuing to " + train.destination);
        metric(x + 24 + column * 2, y + 85, column - 12, "SEGMENT POSITION",
            number(train.segmentPosition / 1000.0) + " / " + number(train.segmentLength / 1000.0, 0) + " km",
            "Departed " + clockTime(train.departure));
        const double progress = train.routePosition / train.routeLength;
        metric(x + 24 + column * 3, y + 85, column - 12, "JOURNEY PROGRESS",
            number(progress * 100.0, 0) + "%",
            number(train.routePosition / 1000.0, 0) + " / " + number(train.routeLength / 1000.0, 0) + " km");
        const int railY = y + 205;
        const int left = x + 53;
        const int right = x + w - 53;
        line(left, railY, right, railY, Track, 4);
        const int trainX = left + static_cast<int>((right - left) * std::clamp(progress, 0.0, 1.0));
        line(left, railY, trainX, railY, Teal, 4);
        for (std::size_t index = 0; index < train.stations.size(); ++index)
        {
            const int center = left + static_cast<int>((right - left) * train.stationDistances[index] / train.routeLength);
            const bool passed = train.routePosition + 0.01 >= train.stationDistances[index];
            stationIcon(center, railY, passed);
            int spacing = right - left;
            if (index > 0)
                spacing = std::min(spacing, static_cast<int>((right - left)
                    * (train.stationDistances[index] - train.stationDistances[index - 1]) / train.routeLength));
            if (index + 1 < train.stations.size())
                spacing = std::min(spacing, static_cast<int>((right - left)
                    * (train.stationDistances[index + 1] - train.stationDistances[index]) / train.routeLength));
            centered(train.stations[index], center, railY + 43, 0, passed ? Ink : Muted, std::max(18, spacing - 12));
        }
        trainIcon(trainX, railY, train.state == TrainState::Waiting ? Amber : Teal);
        line(x + 24, y + 265, x + w - 24, y + 265, Border);
        const auto& parameters = train.parameters;
        const std::string dimensions = "Mass " + number(parameters.mass, 0) + " kg  ·  Length "
            + number(parameters.length, 0) + " m  ·  Max " + number(parameters.maxSpeed)
            + " m/s  ·  Elapsed " + clockTime(train.elapsed);
        const std::string forces = "Engine " + number(parameters.engineForce, 0) + " N  ·  Brake "
            + number(parameters.brakeForce, 0) + " N  ·  Resistance " + number(parameters.friction, 3) + " N·s/m";
        text(dimensions, x + 24, y + 282, 0, Muted, w - 48);
        text(forces, x + 24, y + 300, 0, Muted, w - 48);
    }

    void drawEmpty()
    {
        const int center = width / 2;
        const int y = viewportTop + std::max(80, (viewportHeight() - 235) / 2);
        rounded(center - 105, y - 28, 210, 105, 18, TealLight);
        line(center - 75, y + 39, center + 75, y + 39, Track, 3);
        stationIcon(center - 70, y + 39, true);
        stationIcon(center + 70, y + 39, false);
        trainIcon(center, y + 39, Teal);
        centered(!simulation.hasStarted() ? "Ready for departure" : "All quiet on the network", center, y + 119, 3, Ink);
        centered(!simulation.hasStarted() ? "Start the simulation to follow today's journeys."
            : "Scheduled journeys will appear when they depart.", center, y + 151, 1, Muted, width - 100);
        if (simulation.getUpcoming())
        {
            const auto& next = *simulation.getUpcoming();
            centered("NEXT DEPARTURE  ·  " + clockTime(next.time), center, y + 197, 0, Teal);
            centered(next.name + "   ·   " + next.origin + " → " + next.destination,
                center, y + 222, 1, Ink, width - 120);
        }
        else
            centered("No services in this scenario", center, y + 198, 1, Muted);
    }

    void draw()
    {
        if (!buffer || bufferWidth != width || bufferHeight != height)
        {
            if (canvas)
                XftDrawDestroy(canvas);
            if (buffer)
                XFreePixmap(display, buffer);
            buffer = XCreatePixmap(display, handle, width, height, DefaultDepth(display, DefaultScreen(display)));
            canvas = XftDrawCreate(display, buffer, DefaultVisual(display, DefaultScreen(display)),
                DefaultColormap(display, DefaultScreen(display)));
            if (!canvas)
                throw std::runtime_error("Cannot create dashboard canvas");
            bufferWidth = width;
            bufferHeight = height;
        }
        XSetClipMask(display, graphics, None);
        XftDrawSetClip(canvas, nullptr);
        rectangle(0, 0, width, height, Background);
        drawHeader();
        drawSection();
        scroll = std::clamp(scroll, 0, maximumScroll());
        XRectangle clip{0, static_cast<short>(viewportTop), static_cast<unsigned short>(width),
            static_cast<unsigned short>(viewportHeight())};
        XSetClipRectangles(display, graphics, 0, 0, &clip, 1, Unsorted);
        XftDrawSetClipRectangles(canvas, 0, 0, &clip, 1);
        if (simulation.getTrains().empty())
            drawEmpty();
        else
        {
            int y = viewportTop - scroll;
            for (const auto& train : simulation.getTrains())
            {
                if (y + cardHeight >= viewportTop && y < height - 37)
                    drawCard(train, y);
                y += cardHeight + cardGap;
            }
        }
        XSetClipMask(display, graphics, None);
        XftDrawSetClip(canvas, nullptr);
        if (maximumScroll() > 0)
        {
            const int thumbHeight = std::max(32, viewportHeight() * viewportHeight() / contentHeight());
            const int thumbY = viewportTop + scroll * (viewportHeight() - thumbHeight) / maximumScroll();
            rounded(width - 24, viewportTop, 6, viewportHeight(), 3, Border);
            rounded(width - 24, thumbY, 6, thumbHeight, 3, Track);
        }
        rectangle(0, height - 28, width, 28, Background);
        circle(34, height - 14, 3, simulation.isRunning() ? Teal : Muted);
        text(simulation.isRunning() ? "Live" : (simulation.hasStarted() ? "Paused" : "Ready"), 46, height - 9, 0, Muted);
        text(std::to_string(simulation.getServiceCount()) + " daily services", 119, height - 9, 0, Muted);
        const std::string shortcuts = "Space: start / pause    ·    1–5: speed    ·    Scroll: more trains";
        text(shortcuts, width - textWidth(shortcuts, 0) - 30, height - 9, 0, Muted);
        XCopyArea(display, buffer, handle, graphics, 0, 0, width, height, 0, 0);
        XFlush(display);
    }
};

ApplicationWindow::ApplicationWindow() : implementation(std::make_unique<Implementation>())
{
    auto& state = *implementation;
    state.display = XOpenDisplay(nullptr);
    if (!state.display)
        throw std::runtime_error("Cannot open an X11 display");
    const int screen = DefaultScreen(state.display);
    state.handle = XCreateSimpleWindow(state.display, RootWindow(state.display, screen),
        60, 40, state.width, state.height, 0, 0, WhitePixel(state.display, screen));
    state.graphics = XCreateGC(state.display, state.handle, 0, nullptr);
    state.initializeGraphics();
    XStoreName(state.display, state.handle, "Railway Network | Live services");
    const auto pid = static_cast<unsigned long>(getpid());
    XChangeProperty(state.display, state.handle, XInternAtom(state.display, "_NET_WM_PID", False),
        XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<const unsigned char*>(&pid), 1);
    XSizeHints hints{};
    hints.flags = PMinSize;
    hints.min_width = 1000;
    hints.min_height = 640;
    XSetWMNormalHints(state.display, state.handle, &hints);
    XSelectInput(state.display, state.handle, StructureNotifyMask | ExposureMask | KeyPressMask
        | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | LeaveWindowMask | FocusChangeMask);
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
    auto previous = std::chrono::steady_clock::now();
    bool dirty = true;
    while (true)
    {
        const auto now = std::chrono::steady_clock::now();
        dirty = state.simulation.advance(std::chrono::duration<double>(now - previous).count()) || dirty;
        previous = now;
        while (XPending(state.display))
        {
            XEvent event;
            XNextEvent(state.display, &event);
            if (event.type == ClientMessage && event.xclient.message_type == state.protocols
                && event.xclient.format == 32 && static_cast<Atom>(event.xclient.data.l[0]) == state.closeMessage)
                return;
            if (event.type == DestroyNotify && event.xdestroywindow.window == state.handle)
            {
                state.handle = 0;
                return;
            }
            if (event.type == ConfigureNotify)
            {
                state.width = std::max(1, event.xconfigure.width);
                state.height = std::max(1, event.xconfigure.height);
            }
            else if (event.type == KeyPress)
            {
                const auto key = XLookupKeysym(&event.xkey, 0);
                if (key == XK_Escape)
                    return;
                if (key == XK_space || key == XK_Return || key == XK_KP_Enter)
                    state.simulation.togglePlayback();
                else if (key >= XK_1 && key <= XK_5)
                    state.simulation.setTimeScale(Implementation::speeds[key - XK_1]);
                else if (key == XK_Page_Down || key == XK_Down)
                    state.scroll += state.viewportHeight() / 2;
                else if (key == XK_Page_Up || key == XK_Up)
                    state.scroll -= state.viewportHeight() / 2;
                else if (key == XK_Home)
                    state.scroll = 0;
                else if (key == XK_End)
                    state.scroll = state.maximumScroll();
            }
            else if (event.type == ButtonPress)
            {
                if (event.xbutton.button == Button4)
                    state.scroll -= 82;
                else if (event.xbutton.button == Button5)
                    state.scroll += 82;
                else if (event.xbutton.button == Button1)
                    state.pressed = state.controlAt(event.xbutton.x, event.xbutton.y);
            }
            else if (event.type == ButtonRelease && event.xbutton.button == Button1)
            {
                if (state.pressed != 0 && state.pressed == state.controlAt(event.xbutton.x, event.xbutton.y))
                    state.activate(state.pressed);
                state.pressed = 0;
            }
            else if (event.type == MotionNotify)
            {
                const int hovered = state.controlAt(event.xmotion.x, event.xmotion.y);
                if (hovered == state.hovered)
                    continue;
                state.hovered = hovered;
            }
            else if (event.type == LeaveNotify || event.type == FocusOut)
            {
                state.hovered = 0;
                state.pressed = 0;
            }
            dirty = true;
        }
        if (dirty)
        {
            state.draw();
            dirty = false;
        }
        pollfd connection{ConnectionNumber(state.display), POLLIN, 0};
        if (poll(&connection, 1, state.simulation.isRunning() ? 16 : -1) < 0 && errno != EINTR)
            throw std::runtime_error("Cannot wait for window events");
        if (connection.revents & (POLLERR | POLLHUP | POLLNVAL))
            throw std::runtime_error("Display connection closed");
    }
}
