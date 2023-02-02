#include stdio.h

#include string.h

#include stdlib.h

#include wiringPi.h

#include wiringSerial.h

#include softTone.h

#include netinetin.h

#include systypes.h

#include syssocket.h

#include pthread.h



#define MAX 80

#define PORT 7755



int flag = 0;


/*case0: 어플에서 분사 off를 눌렀을 때, case1: 어플에서 분사 on을 눌렀을 때*/
void send_command(int fd, int onoff)

{

	switch (onoff)

	{

		case 0

			serialPutchar(fd, 0x7A);

			serialPutchar(fd, 0x70);

			serialPutchar(fd, 0x7F);

			serialPutchar(fd, 0x7E);

			break;

			case 1

				serialPutchar(fd, 0x7A);

				serialPutchar(fd, 0x71);

				serialPutchar(fd, 0x7F);

				serialPutchar(fd, 0x7E);

				break;

	}

}


/*부저 모듈 작동*/
int buz(void)

{

	const int pinPiezo = 13;

	const int aMelody[8] = { 523,659,784,659,523 };



	wiringPiSetupGpio();

	softToneCreate(pinPiezo);



	int i;

	for (i = 0; i8; i++)

	{

		softToneWrite(pinPiezo, aMelody[i]);

		delay(250);

	}

	return 0;

}



nPiezo = 13;

const int aMelody[8] = { 523,659,784,659,523 };



wiringPiSetupGpio();

softToneCreate(pinPiezo);



int i;

for (i = 0; i8; i++)

{

	softToneWrite(pinPiezo, aMelody[i]);

	delay(250);

}

return 0;

}



void rx_thread(void arg)

{

	int socket = (int)arg;

	char buff[MAX];



	while (1)

	{

		read(socket, buff, 4);

		buff[4] = 0;



		if (strncmp(buff, 1111, 4) == 0)

		{

			flag = 1;

			printf(ONn);

		}

		else if (strncmp(buff, 2222, 4) == 0)

		{

			flag = 0;

			printf(OFFn);

		}

	}

}



void data_transfer(int socket, double C7H8, double NH3, double HCHO)

{

	char buff[MAX];

	sprintf(buff, %.2lf %.2lf %.2lf, C7H8, NH3, HCHO);

	write(socket, buff, strlen(buff));

}



int main(void)

{

	int fd;

	char c;

	char data[128];

	int Tdec, Tpoi;

	int Ndec, Npoi;

	int Hdec, Hpoi;

	int idx;

	double C7H8, NH3, HCHO;


/*소켓 통신 - server*/
	int server_socket, client_socket, len;

	int opt = 1;

	struct sockaddr_in server_info, client_info;

	pthread_t rx;



	fd = serialOpen(devttyACM0, 9600);

	wiringPiSetup();



	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	server_info.sin_family = AF_INET;

	server_info.sin_port = htons(PORT);

	server_info.sin_addr.s_addr = htonl(INADDR_ANY);

	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if (bind(server_socket, (struct sockaddr)&server_info, sizeof(server_info)) != 0)

	{

		printf(socket bind failedn);

		exit(0);

	}

	if (listen(server_socket, 5) != 0)

	{

		printf(Listen failedn);

		exit(0);

	}

	len = sizeof(client_info);

	client_socket = accept(server_socket, (struct sockaddr)&client_info, &len);

	if (client_socket  0)

	{

		printf(Accept errorn);

		exit(0);

	}

	else

	{

		printf(Accept donen);

		pthread_create(&rx, NULL, rx_thread, (void)&client_socket);

	}




	/*아두이노에서 받아온 센서값을 서버 화면에 출력*/
	while (1)

	{

		c = serialGetchar(fd);

		if (c == 0x1A)

		{

			idx = 0;

			while ((c = serialGetchar(fd)) != 0x1E)

			{

				switch (idx)

				{

					case 0

						Tdec = c;

						break;

						case 1

							Tpoi = c;

							break;

							case 2

								Ndec = c;

								break;

								case 3

									Npoi = c;

									break;

									case 4

										Hdec = c;

										break;

										case 5

											Hpoi = c;

											break;
				}
				idx += 1;
			}
			sprintf(data, %d. % 02d, %d. % 02d, %d. % 02d, Tdec, Tpoi, Ndec, Npoi, Hdec, Hpoi);  C7H8, N
				printf(%sn, data);
			C7H8 = (double)Tdec + (double)Tpoi  100;
			NH3 = (double)Ndec + (double)Npoi  100;
			HCHO = (double)Hdec + (double)Hpoi  100;
			data_transfer(client_socket, C7H8, NH3, HCHO);

			/*flag 1이 들어올 때(어플에서 분사 on을 눌렀을 때)와 그렇지 않을 때 분사 코드*/
			if (flag == 1)
			{
				send_command(fd, 1);
			}
			else
			{
				if (C7H8 > 0.25 || NH3 > 0.02 || HCHO > 0.1)
				{
					printf(AUTO SPRAYn);
					send_command(fd, 1);
					buz();
				}
				else
				{
					send_command(fd, 0);
				}
			}
		}
	}
}