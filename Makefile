NAME = network
LIB_NAME = libftpp.a
LIB_PATH = $(LIB_DIR)/$(LIB_NAME)

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = inc
LIB_DIR = lib

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -I$(INC_DIR) -I$(LIB_DIR)/include -I$(LIB_DIR)/template

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))
DEP = $(wildcard $(INC_DIR)/*.hpp)

RED 	= \033[0;31m
GREEN	= \033[0;32m
YELLOW  = \033[0;33m
BLUE 	= \033[0;34m
MAGENTA = \033[0;35m
CYAN    = \033[0;36m
GRAY	= \033[0;37m
RESET   = \033[0m

all: $(NAME)

$(LIB_PATH):
	@printf "$(YELLOW)[LIB] Building library$(RESET)\n"
	@$(MAKE) -C $(LIB_DIR)

$(NAME): $(LIB_PATH) $(OBJ) 
	@printf "$(MAGENTA)[${NAME}] linking executable$(RESET)\n"
	@$(CXX) $(CXXFLAGS) -no-pie -o $(NAME) $(OBJ) $(LIB_PATH) 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP)
	@mkdir -p $(OBJ_DIR)
	@printf "$(MAGENTA)[${NAME}] compile $<$(RESET)\n"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@printf "$(YELLOW)[${NAME}] clean objects$(RESET)\n"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@printf "$(YELLOW)[${NAME}] remove $(NAME)$(RESET)\n"
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re