#define _BSD_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <curl/curl.h>

#include <X11/Xlib.h>

char *tzgmt = "GMT";

static Display *dpy;

char *
smprintf(char *fmt, ...)
{
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);

	ret = malloc(++len);
	if (ret == NULL) {
		perror("malloc");
		exit(1);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);

	return ret;
}
/*
void
settz(char *tzname)
{
	setenv("TZ", tzname, 1);
}*/

char *
mktimes(char *fmt, char *tzname)
{
	char buf[129];
	time_t tim;
	struct tm *timtm;

	memset(buf, 0, sizeof(buf));
//	settz(tzname);
	tim = time(NULL);
	timtm = localtime(&tim);
	if (timtm == NULL) {
		perror("localtime");
		exit(1);
	}

	if (!strftime(buf, sizeof(buf)-1, fmt, timtm)) {
		fprintf(stderr, "strftime == 0\n");
		exit(1);
	}

	return smprintf("%s", buf);
}

void
setstatus(char *str)
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

char *
loadavg(void)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) {
		perror("getloadavg");
		exit(1);
	}

	return smprintf("%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
}

//char *
//serverload(void) {
//  double savgs[3];
//
//  CURL *curl;
//  CURLcode res;
//
//  curl = curl_easy_init();
//
//  if(!curl) return 0;
//
//  curl_eae


char *
batery(void){
  char percent[4];
  FILE *cap;

  cap = fopen("/sys/class/power_supply/sbs-5-000b/capacity", "r");
  fgets(percent, 4, (FILE*)cap);
  strtok(percent, "\n");

  fclose(cap);

  if (strcmp(percent, "100") == 0 ) {
    strcpy(percent, "Full");
  } else {
    strcpy(percent, strcat(percent, "%"));
  }

  return smprintf("%s",percent);
}

char *
bright(void) {
  char bri[3];
  FILE *cap;

  cap = fopen("/sys/class/backlight/pwm-backlight/brightness", "r");
  fgets(bri, 3, (FILE*)cap);
  strtok(bri, "\n");

  fclose(cap);

  return smprintf("%s",bri);
}


int
main(void)
{
	char *status;
	char *avgs;
	char *tmgmt;
	char *bat;
  char *bri;

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	for (;;sleep(1)) {
		avgs = loadavg();
		tmgmt = mktimes("%A %d %B %H:%M:%S", tzgmt);
    bat = batery();
    bri = bright();

		status = smprintf("%s | %s | %s | %s",
				bri, bat, avgs, tmgmt);
		setstatus(status);

    free(bri);
    free(bat);
		free(avgs);
		free(tmgmt);
		free(status);
	}

	XCloseDisplay(dpy);

	return 0;
}

