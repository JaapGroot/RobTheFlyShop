#ifndef libcaptcha_h
#define libcaptcha_h

const int gifsize;


static int8_t *lt[];
const int gifsize=17646;


//make the shit for the captcha
void captcha(unsigned char im[70*200], unsigned char l[6]);
void makegif(unsigned char im[70*200], unsigned char gif[gifsize]);



#endif
