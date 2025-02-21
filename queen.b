/* 0:t 1:l 2:c 3:r 4:m */

depth 12;	/* max. 12 */
d[96];		/* 12*8 */
n[5];		/* max. 5digits */
one 1;		/* avoid compiler bug */

output() {
	extrn putchar;
	extrn n, depth, d, one;
	auto i 0, j;
	putchar(27); putchar('['); putchar(';'); putchar('H');
	while (++n[i] > 9)
		n[i++] = 0;
	i = 5;
	while (i)
		putchar('0' + n[--i]);
	putchar('*n');
	i = 0;
	while (i < depth) {
		j = 0;
		while (j < depth) {
			putchar(' ');
			putchar(d[(i << 3) + 4] & one << j ? 'Q' : '**');
			j++;
		}
		putchar('*n');
		i++;
	}
}

main() {
	extrn putchar, output;
	extrn d, depth, one;
	auto m, p, p0, mask;
	putchar(27); putchar('['); putchar('2'); putchar('J');
	mask = (one << depth) - 1;
	p = d;
	*p = mask;
	*(p + 1) = *(p + 2) = *(p + 3) = *(p + 4) = 0;
	while (1) {
		if (*p) {
			*p =& ~(*(p + 4) = m = *p & -*p);
			if (p - d < depth - 1 << 3) {
				p0 = p;
				p =+ 8;
				*(p + 1) = (*(p0 + 1) | m) << 1;
				*(p + 2) =  *(p0 + 2) | m;
				*(p + 3) = (*(p0 + 3) | m) >> 1;
				*p = ~(*(p + 1) | *(p + 2) | *(p + 3)) & mask;
			}
			else output();
		}
		else if ((p =- 8) < d) break;
	}
}

