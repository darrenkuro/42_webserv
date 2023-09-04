NAME = webserv

# Source files
_DIR	:=	http utils
_SRC 	:=	http/HttpRequest.cpp http/HttpResponse.cpp Webserver.cpp Server.cpp Client.cpp \
			ConfigParser.cpp utils/utils.cpp main.cpp
_INC	:=	http/HttpRequest.hpp http/HttpResponse.hpp Webserver.hpp Server.hpp Client.hpp \
			ConfigParser.hpp utils/utils.hpp

OBJDIR	:=	obj
SRCDIR	:=	src
SRC		:=	$(addprefix $(SRCDIR)/, $(_SRC))
INC		:=	$(addprefix $(SRCDIR)/, $(_INC))
OBJ		:=	$(addprefix $(OBJDIR)/, $(_SRC:.cpp=.o))
CXX		:=	c++
RM		:=	/bin/rm -rf

CXXFLAGS	:=	-Wall -Werror -Wextra -std=c++98
INCFLAGS		:=	-I $(SRCDIR) -I $(SRCDIR)/http -I $(SRCDIR)/utils

all: $(NAME)

$(NAME): $(OBJ) $(INC)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(INC)
	@mkdir -p $(OBJDIR) && cd $(OBJDIR) && mkdir -p $(_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCFLAGS)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re : fclean all

.PHONY : all clean fclean re

sim:
	make
	make -C client_spawner
	client_spawner/client_spawner &
	./webserv
