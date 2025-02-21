w 64;
h 23;
buf0[184]; /* 64 / 8 * 23 */
buf1[184];
o;
n;
bitmask[8];

main() {
	extrn init, update;
	init();
	while (1)
		update();
}

get(x, y) {
	extrn o, bitmask;
	auto p, m;
	p = o + (y << 3 | x >> 3); /* w==64 */
	m = bitmask[x & 7];
	return (*p & m) != 0;
}

put(x, y, r) {
	extrn n, bitmask;
	auto p, m;
	p = n + (y << 3 | x >> 3); /* w==64 */
	m = bitmask[x & 7];
	if (r & 1) *p = *p | m;
	else *p = *p & ~m;
}

init() {
	extrn o, n, h, w, buf0, buf1, bitmask;
	extrn rand, put, putchar;
	auto x, y;
	auto i 0, m 1;
	while (i < 8) {
		bitmask[i++] = m;
		m =<< 1;
	}
	o = buf0;
	n = buf0;
	y = 0;
	while (y < h) {
		x = 0;
		while (x < w) {
			rand();
			put(x, y, *07770);
			x++;
		}
		y++;
	}
	n = buf1;
	putchar(27); putchar('['); putchar('2'); putchar('J');
}

/* 07770-07772 random work */

rand() {
	*07772 = (*07772 ^ (*07771 << 1 | *07770 >> 11)) & 0377;
	*07771 =^ *07770 << 1;

	*07770 =^ *07772 << 7 | *07771 >> 5;
	*07771 =^ *07772 >> 5;

	*07772 = (*07772 ^ (*07772 << 5 | *07771 >> 7)) & 0377;
	*07771 =^ *07771 << 5 | *07770 >> 7;
	*07770 =^ *07770 << 5;
}

count(i, j) {
	extrn get, w, h;
	auto c 0, u, v, x, y;
	v = j - 1;
	while (v != j + 2) {
		u = i - 1;
		while (u != i + 2) {
			if (u != i | v != j) {
				x = u;
				if (x & 04000) x =+ w;
				if (x >= w) x =- w;
				y = v;
				if (y & 04000) y =+ h;
				if (y >= h) y =- h;
				if (get(x, y)) c++;
			}
			u++;
		}
		v++;
	}
	return (c);
}

update() {
	extrn w, h, o, n, get, putchar, count, put;
	auto i, j, c, r, t;
	putchar(27); putchar('['); putchar(';'); putchar('H');
	j = 0;
	while (j < h) {
		i = 0;
		while (i < w) {
			c = count(i, j);
			r = 0;
			if (c == 2) r = get(i, j);
			if (c == 3) r = 1;
			put(i, j, r);
			putchar(r ? '**' : '.');
			i++;
		}
		putchar('*n');
		j++;
	}
	t = o;
	o = n;
	n = t;
}

putnum(i) {
	extrn putchar;
	putchar('0' + (i >> 9));
	putchar('0' + (i >> 6 & 7));
	putchar('0' + (i >> 3 & 7));
	putchar('0' + (i & 7));
}

