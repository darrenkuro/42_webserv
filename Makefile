NAME = webserv

# Source files
_SRC 	:=	Webserv.cpp HttpParser.cpp utils.cpp main.cpp
_INC	:=	Webserv.hpp HttpParser.hpp utils.hpp

OBJDIR	:=	obj
SRCDIR	:=	src
SRC		:=	$(addprefix $(SRCDIR)/, $(_SRC))
INC		:=	$(addprefix $(SRCDIR)/, $(_INC))
OBJ		:=	$(addprefix $(OBJDIR)/, $(_SRC:.cpp=.o))
CXX		:=	c++
RM		:=	/bin/rm -rf

CXXFLAGS	:=	-Wall -Werror -Wextra -std=c++98

all: $(NAME)

$(NAME): $(OBJ) $(INC)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(INC)
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $< -I $(SRCDIR)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re : fclean all

.PHONY : all clean fclean re