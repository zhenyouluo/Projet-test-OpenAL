NAME = OpenAL_test

FLG =	-Wall -Wextra -Werror

SRC =	main.cpp \

#INC =	-I./inc \

all: $(NAME)

$(NAME):
	g++ $(FLG) $(SRC) $(INC) -lalut `pkg-config --libs --cflags sndfile openal` -o $(NAME)

clean:
	rm -f $(NAME)

re: clean all
