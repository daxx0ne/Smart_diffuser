#include <MQUnifiedsensor.h>

//Definitions
#define placa "Arduino UNO"
#define Voltage_Resolution 5
#define pin A0 //Analog input 0 of your arduino
#define type "MQ-135" //MQ135
#define ADC_Bit_Resolution 10 // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6//RS / R0 = 3.6 ppm  
//#define calibration_button 13 //Pin to calibrate your sensor

//LCD
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Declare Sensor
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

void setup() {
	pinMode(PD2, OUTPUT);
	Serial.begin(9600); //Init serial port
	MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
	MQ135.init();

	// I2C LCD�� �ʱ�ȭ
	lcd.begin();
	// I2C LCD�� �����Ʈ Ű��
	lcd.backlight();

	float calcR0 = 0;
	for (int i = 1; i <= 10; i++)
	{
		MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
		calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
	}
	MQ135.setR0(calcR0 / 10);


	if (isinf(calcR0)) { Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while (1); }
	if (calcR0 == 0) { Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while (1); }

}

void loop() {
	MQ135.update(); // Update data, the arduino will read the voltage from the analog pin

					// Configure the equation to calculate Toluen concentration value
	float Toluen = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

									   // Configure the equation to calculate NH4 concentration value
	float NH3 = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

									// Configure the equation to calculate Aceton concentration value
	float HCHO = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

	int num_sample = 10;

	float Tsum = 0.0;
	float Tavg = 0.0;
	for (int i = 0; i<num_sample; i++) { //���ø��ϱ�
		MQ135.setA(44.947); MQ135.setB(-4.445);
		Toluen = MQ135.readSensor();
		Tsum = Tsum + Toluen;
	}
	Tavg = Tsum / num_sample;
	//Serial.print("   |   "); Serial.print(Tavg);

	float Nsum = 0.0;
	float Navg = 0.0;
	for (int j = 0; j<num_sample; j++) {
		MQ135.setA(41.2121); MQ135.setB(-6.812);
		NH3 = MQ135.readSensor();
		Nsum = Nsum + NH3;
	}
	Navg = Nsum / num_sample;
	//Serial.print("   |   "); Serial.print(Navg);

	float Hsum = 0.0;
	float Havg = 0.0;
	for (int k = 0; k<num_sample; k++) {
		MQ135.setA(34.668); MQ135.setB(-4.969);
		HCHO = MQ135.readSensor();
		Hsum = Hsum + HCHO;
	}
	Havg = Hsum / num_sample;
	//Serial.print("   |   "); Serial.print(Havg);
	//Serial.println("   |");


	//float Tavg = Toluen;
	//float Navg = NH3;
	//float Havg = HCHO;

	char msg[128] = { 0 };
	int Tdec, Tpoi;
	int Ndec, Npoi;
	int Hdec, Hpoi;
	//sprintf(msg, "%s,%s,%s\n", String(Tavg, 2).c_str(), String(Navg, 2).c_str(), String(Havg, 2).c_str());
	//Serial.println(msg);

	//msg[strlen(msg + 1)] = 0x1E;
	// 0.01,0.01,0.01
	//Serial.println(Navg);
	Tdec = (int)Tavg; //����, �Ǽ��κ� ������
	Tpoi = Tavg * 100 - Tdec;
	Ndec = (int)Navg;
	Npoi = Navg * 100 - Ndec;
	Hdec = (int)Havg;
	Hpoi = Havg * 100 - Hdec;
	// 0 = 1A
	msg[0] = 0x1A; //����
	msg[1] = Tdec; //�Ǽ�~���� ���� �ޱ�
	msg[2] = Tpoi;
	msg[3] = Ndec;
	msg[4] = Npoi;
	msg[5] = Hdec;
	msg[6] = Hpoi;
	msg[7] = 0x1E; //����

	 /*������ ���� ��� ����*/
	double C7H8, AMMONIA, FORMALDEHYDE;

	C7H8 = (double)Tdec + (double)Tpoi / 100; //�Ǽ�, ���� �κ� ��ġ��
	AMMONIA = (double)Ndec + (double)Npoi / 100;
	FORMALDEHYDE = (double)Hdec + (double)Hpoi / 100;

	//sprintf(msg, "%d.%d,  %d.%d,  %d.%d", msg[1], msg[2], msg[3], msg[4], msg[5], msg[6]);
	//Serial.println(msg);
	Serial.write(msg, 8); //msg ������
	delay(1000);

	/*LCD ��� ������ ����ġ ����*/
	if (C7H8 > 0.25 || NH3 > 0.02 || HCHO > 0.1) //����ġ���� ���� ��
	{
		lcd.print("  State: ");
		lcd.print("BAD");
		delay(1000);
		lcd.clear();
	}

	else if (C7H8 < 0.16 && NH3 < 0.01 && HCHO < 0.07) //����ġ���� ��ġ�� ���� ���� ��
	{
		lcd.print("  State: "); //������ ����մϴ�.
		lcd.print("GOOD");
		// 1�ʰ� ����մϴ�.
		delay(1000);
		// LCD�� ��� ������ �����մϴ�.
		lcd.clear();
	}

	else
	{
		lcd.print("  State: "); //������ ����մϴ�. //����~������ ����
		lcd.print("NORMAL");
		// 1�ʰ� ����մϴ�.
		delay(1000);
		// LCD�� ��� ������ �����մϴ�.
		lcd.clear();
	}
}


void serialEvent() //rx�� ������ ����Ǵ� �Լ� (�л�)
{
	unsigned char cmd[4];

	if (Serial.available()) //������ �� ���
	{
		Serial.readBytes(cmd, 4);
		if (cmd[0] == 0x7A && cmd[3] == 0x7E)
		{
			switch (cmd[1])
			{
			case 0x70: //�л� X
				digitalWrite(PD2, 0);
				break;
			case 0x71: //�л� O
				digitalWrite(PD2, 1);
				break;
			}
		}
	}
}