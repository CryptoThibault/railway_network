NAME = network
SRCS = src/main.cpp src/Loader.cpp src/Simulation.cpp src/Network.cpp src/Segment.cpp src/Train.cpp \
	   src/Motion.cpp src/Board.cpp src/Logger.cpp src/Printer.cpp src/Traveler.cpp
OBJS = $(SRCS:.cpp=.o)
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -Iinclude
all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re