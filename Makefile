NAME = network
SRCS = main.cpp Loader.cpp Simulation.cpp Network.cpp  Segment.cpp Train.cpp  \
	   Motion.cpp Board.cpp Logger.cpp Printer.cpp Traveler.cpp
OBJS = $(SRCS:.cpp=.o)
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror
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