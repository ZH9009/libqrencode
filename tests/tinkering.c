#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "common.h"
#include "../qrencode.h"
#include "decoder.h"

static int margin = 4;
static int size = 3;
static unsigned char fg_color[4] = {0, 0, 0, 255};
static unsigned char bg_color[4] = {255, 255, 255, 255};

static int writeXPM(const QRcode *qrcode, FILE *fp)
{
	int x, xx, y, yy, realwidth, realmargin;
	char *row;
	char fg[7], bg[7];
	unsigned char *p;

	realwidth = (qrcode->width + margin * 2) * size;
	realmargin = margin * size;

	row = malloc(realwidth + 1);
	if (!row ) {
		fprintf(stderr, "Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	snprintf(fg, 7, "%02x%02x%02x", fg_color[0], fg_color[1],  fg_color[2]);
	snprintf(bg, 7, "%02x%02x%02x", bg_color[0], bg_color[1],  bg_color[2]);

	fputs("/* XPM */\n", fp);
	fputs("static const char *const qrcode_xpm[] = {\n", fp);
	fputs("/* width height ncolors chars_per_pixel */\n", fp);
	fprintf(fp, "\"%d %d 2 1\",\n", realwidth, realwidth);

	fputs("/* colors */\n", fp);
	fprintf(fp, "\"F c #%s\",\n", fg);
	fprintf(fp, "\"B c #%s\",\n", bg);

	fputs("/* pixels */\n", fp);
	memset(row, 'B', realwidth);
	row[realwidth] = '\0';

	for (y = 0; y < realmargin; y++) {
		fprintf(fp, "\"%s\",\n", row);
	}

	p = qrcode->data;
	for (y = 0; y < qrcode->width; y++) {
		for (yy = 0; yy < size; yy++) {
			fputs("\"", fp);

			for (x = 0; x < margin; x++) {
				for (xx = 0; xx < size; xx++) {
					fputs("B", fp);
				}
			}

			for (x = 0; x < qrcode->width; x++) {
				for (xx = 0; xx < size; xx++) {
					if (p[(y * qrcode->width) + x] & 0x1) {
						fputs("F", fp);
					} else {
						fputs("B", fp);
					}
				}
			}

			for (x = 0; x < margin; x++) {
				for (xx = 0; xx < size; xx++) {
					fputs("B", fp);
				}
			}

			fputs("\",\n", fp);
		}
	}

	for (y = 0; y < realmargin; y++) {
		fprintf(fp, "\"%s\"%s\n", row, y < (size - 1) ? "," : "};");
	}

	free(row);

	return 0;
}

void tinkering_invalid_terminator()
{
	QRinput *input;
	QRcode *qrcode;
	QRdata *qrdata;
	char *str = "I'm so happy.";
	char *secret_str = "[SECRET TEXT]";
	int ret;
	FILE *fp;

	input = QRinput_new2(5, QR_ECLEVEL_H);
	ret = QRinput_append(input, QR_MODE_8, strlen(str), (unsigned char *)str);
	if(ret) fprintf(stderr, "Failed to encode 1st string.\n");
	ret = QRinput_append(input, QR_MODE_NUL, 0, NULL);
	if(ret) fprintf(stderr, "Failed to encode the terminator.\n");
	ret = QRinput_append(input, QR_MODE_8, strlen(secret_str), (unsigned char *)secret_str);
	if(ret) fprintf(stderr, "Failed to encode 2nd string.\n");

	qrcode = QRcode_encodeInput(input);
	if(qrcode != NULL) {
		fp = fopen("tinkering_TERM.xpm", "w");
		writeXPM(qrcode, fp);
		fclose(fp);
	}
	qrdata = QRcode_decode(qrcode);
	if(qrdata == NULL) {
		fprintf(stderr, "Something wrong.\n");
	} else {
		fprintf(stderr, "decoding\n");
		QRdata_dump(qrdata);
	}

	if(qrdata != NULL) QRdata_free(qrdata);
	if(qrcode != NULL) QRcode_free(qrcode);
}

void tinkering_NUL()
{
	QRinput *input;
	QRcode *qrcode;
	QRdata *qrdata;
	char str[] = "I'm so happy.\0[SECRET TEXT]";
	int ret;
	FILE *fp;

	input = QRinput_new2(0, QR_ECLEVEL_H);
	ret = QRinput_append(input, QR_MODE_8, 27, (unsigned char *)str);
	if(ret) fprintf(stderr, "Failed to encode data including NUL.\n");

	qrcode = QRcode_encodeInput(input);
	if(qrcode != NULL) {
		fp = fopen("tinkering_NUL.xpm", "w");
		writeXPM(qrcode, fp);
		fclose(fp);
	}
	qrdata = QRcode_decode(qrcode);
	if(qrdata == NULL) {
		fprintf(stderr, "Something wrong.\n");
	} else {
		fprintf(stderr, "decoding\n");
		QRdata_dump(qrdata);
	}

	if(qrdata != NULL) QRdata_free(qrdata);
	if(qrcode != NULL) QRcode_free(qrcode);
}

int main(int argc, char **argv)
{
	tinkering_invalid_terminator();
	tinkering_NUL();

	return 0;
}
