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
  *(s + 1) = '\0';
}


int	main()
{
	const char SAN[]="1. Nf3 Nf6 2. c4 g6 3. Nc3 Bg7 4. d4 O-O 5. Bf4 d5 6. Qb3 dxc4 7. Qxc4 c6 8. e4 Nbd7 9. Rd1 Nb6 10. Qc5 Bg4 11. Bg5 Na4 12. Qa3 Nxc3 13. bxc3 Nxe4 14. Bxe7 Qb6 15. Bc4 Nxc3 16. Bc5 Rfe8+ 17. Kf1 Be6 18. Bxb6 Bxc4+ 19. Kg1 Ne2+ 20. Kf1 Nxd4+ 21. Kg1 Ne2+ 22. Kf1 Nc3+ 23. Kg1 axb6 24. Qb4 Ra4 25. Qxb6 Nxd1 26. h3 Rxa2 27. Kh2 Nxf2 28. Re1 Rxe1 29. Qd8+ Bf8 30. Nxe1 Bd5 31. Nf3 Ne4 32. Qb8 b5 33. h4 h5 34. Ne5 Kg7 35. Kg1 Bc5+ 36. Kf1 Ng3+ 37. Ke1 Bb4+ 38. Kd1 Bb3+ 39. Kc1 Ne2+ 40. Kb1 Nc3+ 41. Kc1 Rc2# 0-1";
	// "1.e4 e5 2.Nf3 Nc6 3.Bc4 Bc5 4.O-O Nf6 5.c3 d6 6.b4 Bb6 7.d3 Bg4 8.a4 a5 9.b5 Ne7 10.h3 Bh5 11.Be3 c6 12.bxc6 bxc6 13.Nbd2 O-O 14.d4 exd4 15.cxd4 d5 16.e5 Ne4 17.Nxe4 dxe4 18.g4 exf3 19.gxh5 Nf5 20.Qxf3 Nxd4 21.Qg4 Kh8 22.Rab1 c5 23.Bg5 Nf3+ 24.Qxf3 Qxg5+ 25.Qg4 Bd8 26.f4 Qh4 27.Rb7 Qxg4+ 28.hxg4 f5 29.g5 Re8 30.Rd1 Re7# 1-0";
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
