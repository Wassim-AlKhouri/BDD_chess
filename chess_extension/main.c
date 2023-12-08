#include <stdio.h>
#include <string.h>
#include"smallchesslib.h"

char str[4096];

void putCharStr(char c)
{
  char *s = str;

  while (*s != 0)
    s++;

  *s = c;
  *(s + 2) = '\0';
}


int	main()
{
	const char SAN[]= "1. e4 d5 2. exd5 Qxd5 3. Nc3 Qd8 4. Bc4 Nf6 5. Nf3 Bg4 6. h3 Bxf3 7. Qxf3 e6 8. Qxb7 Nbd7 9. Nb5 Rc8 10. Nxa7 Nb6 11. Nxc8 Nxc8 12. d4 Nd6 13. Bb5+ Nxb5 14. Qxb5+ Nd7 15. d5 exd5 16. Be3 Bd6 17. Rd1 Qf6 18. Rxd5 Qg6 19. Bf4 Bxf4 20. Qxd7+ Kf8 21. Qd8#";

	SCL_Record	r;
	SCL_recordInit(r);
	SCL_recordFromPGN(r, SAN);
    printf("size = %i\n", SCL_recordLength(r));
	/* SCL_recordRemoveLast(r);
	SCL_recordRemoveLast(r); */
	SCL_printPGN(r, putCharStr, 0);
	printf("str = %s\n", str);
	printf("str = %s\n", str);
	


}
