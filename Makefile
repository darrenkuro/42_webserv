NAME = webserv

# Source files
_SRC 	:=	webserv.cpp
_INC	:=	webserv.hpp error.hpp

OBJDIR	:=	objs
SRCDIR	:=	srcs
INCDIR	:=	incs
SRC		:=	$(addprefix $(SRCDIR)/, $(_SRC))
INC		:=	$(addprefix $(INCDIR)/, $(_INC))
OBJ		:=	$(addprefix $(OBJDIR)/, $(_SRC:.cpp=.o))
CXX		:=	c++
RM		:=	/bin/rm -rf

CXXFLAGS	:=	-Wall -Werror -Wextra -std=c++98

all: $(NAME)

$(NAME): $(OBJ) $(INC)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(INC)
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $< -I $(INCDIR)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re : fclean all

.PHONY : all clean fclean re
