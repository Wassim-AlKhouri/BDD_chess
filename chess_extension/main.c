#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char	*Fomate_SAN(const char *str)
{
	char	*SAN;
	int		nb_extraspace = 0;
	int		nb_left_spaces = 0;
	int		nb_right_spaces = 0;
	int		nb_space_to_add = 0;
	int		i = 0;
	int		j = 0;
	int		SAN_len;
	
	while(str[strlen(str) - 1 - nb_right_spaces] == ' ')
		nb_right_spaces ++; //remove all right spaces
	while(str[nb_left_spaces] == ' ')
		nb_left_spaces++; //remove all left spaces
	while(str[nb_left_spaces + i] && nb_left_spaces + i < strlen(str) - nb_right_spaces){
		if (str[nb_left_spaces + i] == ' ')
		{
			i++;
			while (str[nb_left_spaces + i] == ' '){
				i++;
				nb_extraspace++;
			}
		}
		else if(str[nb_left_spaces + i] == '.'){
			i++;
			if (str[nb_left_spaces + i] != ' ')
				nb_space_to_add++;
		}
		else
			i++;
	}
	SAN_len = strlen(str) - nb_extraspace - nb_left_spaces - nb_right_spaces + nb_space_to_add;
	SAN = malloc(sizeof(char) * (SAN_len + 1));
	i = 0;
	j = nb_left_spaces;
	while(i < SAN_len){
		SAN[i] = str[j];
		if (str[j] == ' '){
			i++;
			j++;
			while (str[j] == ' ')
				j++; //remove extra spaces
		}
		else if (str[j] == '.' && str[j + 1] != ' '){
			SAN[i + 1] = ' ';
			i += 2;
			j++;
		}
		else{
			i++;
			j++;
		}
	}
	SAN[SAN_len] = '\0';
	return (SAN);
}

int	main(){
	const char str[] = "   1. e4   e5 2.e4  ";
	char	*SAN = Fomate_SAN(str);
	printf("XX%sXX\n", SAN);
	free(SAN);
}