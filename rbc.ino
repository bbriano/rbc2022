/* Color sensor pins */
#define S0 0
#define S1 1
#define S2 2
#define S3 3
#define LOUT 12
#define ROUT 13

/* Left wheel pins */
#define ENB 10
#define IN3 8
#define IN4 5

/* Right wheel pins */
#define ENA 9
#define IN1 6
#define IN2 7

#define SPEED 180 /* must be in [0,255] */
#define COLOR_DELAY 50 /* milliseconds */

/* Calibrated color frequencies */
#define NRGB 5 /* NOTE: keep in sync with below */
int LRGB[NRGB][3] = {
	/* Ordered from low to high priority */
	{  11,  20,  17 }, /* white */
	{  16,  30,  26 }, /* black */
	{  13,  29,  24 }, /* red */
	{  12,  21,  21 }, /* yellow */
	{  16,  28,  24 }, /* green */
};
int RRGB[NRGB][3] = {
	/* Ordered from low to high priority */
	{  11,  19,  15 }, /* white */
	{  15,  29,  23 }, /* black */
	{  13,  27,  21 }, /* red */
	{  11,  21,  20 }, /* yellow */
	{  15,  27,  22 }, /* green */
};
char *COLOR_NAME[NRGB] = {
	"WHITE",
	"BLACK",
	"RED",
	"YELLOW",
	"GREEN",
};

void setup() {
	/* Open serial port at 9600 bps. */
	Serial.begin(9600);

	/* Set pin directions. */
	pinMode(ENA, OUTPUT);
	pinMode(IN1, OUTPUT);
	pinMode(IN2, OUTPUT);
	pinMode(ENB, OUTPUT);
	pinMode(IN3, OUTPUT);
	pinMode(IN4, OUTPUT);
	pinMode(S0, OUTPUT);
	pinMode(S1, OUTPUT);
	pinMode(S2, OUTPUT);
	pinMode(S3, OUTPUT);
	pinMode(LOUT, INPUT);
	pinMode(ROUT, INPUT);

	/* Set color sensor frequency scaling to 100%. */
	digitalWrite(S0, HIGH);
	digitalWrite(S1, HIGH);

	/* Set pin 9 & pin 10 PWM frequency to be 3921.16 Hz.
	This allow the wheels to have more torque while keeping RPM low.
	src: https://arduinoinfo.mywikis.net/wiki/Arduino-PWM-Frequency */
	TCCR1B = TCCR1B & B11111000 | B00000010;
}

void loop() {
	int L, R;
	color(&L, &R);

	if (L < R) {
		/* Turn right */
		speed(SPEED, -SPEED);
	} else if (L > R) {
		/* Turn left */
		speed(-SPEED, SPEED);
	} else {
		/* Go forward */
		speed(SPEED, SPEED);
	}
}

/* color returns the color index identified by the color sensor. */
void color(int *left, int *right) {
	/* Red filter */
	digitalWrite(S2, LOW);
	digitalWrite(S3, LOW);
	int LR = pulseIn(LOUT, LOW);
	int RR = pulseIn(ROUT, LOW);
	delay(COLOR_DELAY);

	/* Green filter */
	digitalWrite(S2, HIGH);
	digitalWrite(S3, HIGH);
	int LG = pulseIn(LOUT, LOW);
	int RG = pulseIn(ROUT, LOW);
	delay(COLOR_DELAY);

	/* Blue filter */
	digitalWrite(S2, LOW);
	digitalWrite(S3, HIGH);
	int LB = pulseIn(LOUT, LOW);
	int RB = pulseIn(ROUT, LOW);
	delay(COLOR_DELAY);

	/* Find closest Color */
	int mindist = 999999;
	for (int col=0; col<NRGB; col++) {
		int dist =
			(LR-LRGB[col][0])*(LR-LRGB[col][0]) +
			(LG-LRGB[col][1])*(LG-LRGB[col][1]) +
			(LB-LRGB[col][2])*(LB-LRGB[col][2]);
		if (dist < mindist) {
			mindist = dist;
			*left = col;
		}
	}
	for (int col=0; col<NRGB; col++) {
		int dist =
			(RR-RRGB[col][0])*(RR-RRGB[col][0]) +
			(RG-RRGB[col][1])*(RG-RRGB[col][1]) +
			(RB-RRGB[col][2])*(RB-RRGB[col][2]);
		if (dist < mindist) {
			mindist = dist;
			*right = col;
		}
	}

	/* Print frequencies and classified color */
	Serial.print(LR);
	Serial.print("\t");
	Serial.print(LG);
	Serial.print("\t");
	Serial.print(LB);
	Serial.print("\t");
	Serial.print(COLOR_NAME[*left]);
	Serial.print("\t");
	Serial.print(RR);
	Serial.print("\t");
	Serial.print(RG);
	Serial.print("\t");
	Serial.print(RB);
	Serial.print("\t");
	Serial.print(COLOR_NAME[*right]);
	Serial.print("\n");
}

/* speed sets the speed of the wheels. left and right must be in [-255,255]. */
void speed(int left, int right) {
	digitalWrite(IN3, left>0 ? LOW : HIGH);
	digitalWrite(IN4, left>0 ? HIGH : LOW);
	analogWrite(ENB, abs(left));

	digitalWrite(IN1, right>0 ? HIGH : LOW);
	digitalWrite(IN2, right>0 ? LOW : HIGH);
	analogWrite(ENA, abs(right));
}
