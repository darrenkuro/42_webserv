NAME = webserv

# Source files
_SRC	:=	Webserver.cpp Server.cpp Client.cpp ConfigParser.cpp Cgi.cpp \
			utils.cpp main.cpp
_INC	:=	Webserver.hpp Server.hpp Client.hpp ConfigParser.hpp Cgi.hpp \
			utils.hpp global.hpp

OBJDIR	:=	obj
SRCDIR	:=	src
SRC		:=	$(addprefix $(SRCDIR)/, $(_SRC))
INC		:=	$(addprefix $(SRCDIR)/, $(_INC))
OBJ		:=	$(addprefix $(OBJDIR)/, $(_SRC:.cpp=.o))
CXX		:=	c++
RM		:=	/bin/rm -rf

CXXFLAGS	:=	-Wall -Werror -Wextra -std=c++98
INCFLAGS	:=	-I $(SRCDIR)

all: $(NAME)

$(NAME): $(OBJ) $(INC)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(INC)
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCFLAGS)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re

sim:
	make
	make -C client_spawner
	client_spawner/client_spawner &
	./webserv
