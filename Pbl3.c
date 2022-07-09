#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <lcd.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <linux/i2c-dev.h>
#include <sys/queue.h>
#include <mosquitto.h>


#define MAXTIMINGS	85
#define DHTPIN		1

#define BUTTON_UP 25
#define BUTTON_SELECT 24
#define BUTTON_DOWN 21

#define LEVER_MENU 7
#define LEVER_TIME 0
#define LEVER_HISTORY 2 
#define LEVER_EXIT 3

#define HOST "10.0.0.101"
#define PORT  1883
#define KEEP_ALIVE 60
#define MSG_MAX_SIZE  512
#define USER "aluno"
#define PASSWORD "aluno*123"

bool session = true;



typedef struct sensor_data{
	double temp, hum, lum, press;
	char *time_stamp;
}sensors;


typedef struct dht11Data{
	double temp;
	double hum;
}dht11;

typedef struct i2c{
	uint16_t sensor_1;
	uint16_t sensor_2;
} i2c_data;


enum {Menu = 0, History, Time, Exit} state;

//Pin's interrupts
void increase();
void decrease();
void wait_command();


void writesData(unsigned char *data, int lcdHandler);
void printSensorData(int lcdHandler, sensors *sensor_data);
void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);
void send_data(struct mosquitto *mosq);
void my_connect_callback(struct mosquitto *mosq, void *userdata, int result);
void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos);
void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str);
double convert_data(double value, int factor);

int initialize_lcd();
int initialize_mqtt();
dht11 *read_dht11_dat();
i2c_data *i2c_comns();




sensors *history[10];
unsigned int interval = 1;
unsigned int hist_size = 0;
unsigned int hist_posi = 0;
int lcd;


int main(){
	//MQTT
	struct mosquitto *mosq = NULL;
	char buff[MSG_MAX_SIZE];
	
    //libmosquitto library initialization 
    mosquitto_lib_init();
    //create mosquitto client
    mosq = mosquitto_new(NULL,session,NULL);
    if(!mosq){
        printf("create client failed..\n");
        mosquitto_lib_cleanup();
        return -1;
    }
    //set the callback function can be used if desired 
    mosquitto_log_callback_set(mosq, my_log_callback);
    mosquitto_connect_callback_set(mosq, my_connect_callback);
    mosquitto_message_callback_set(mosq, my_message_callback);
    mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
    mosquitto_username_pw_set(mosq ,USER, PASSWORD);
	

    //server connection 
    if(mosquitto_connect(mosq, HOST, PORT, KEEP_ALIVE)){
        fprintf(stderr, "Unable to connect.\n");
        return -1;
    }
    int loop = mosquitto_loop_start(mosq); 
    if(loop != MOSQ_ERR_SUCCESS)
    {
        printf("mosquitto loop error\n");
        return 1;
    }
    printf("---------------PASSOU---------------");
    //LCD
	dht11 *data = NULL;
	i2c_data *data_i2c = NULL;
	//Init WiringPi	and lcd display 
	if(	wiringPiSetup() == -1) return -1;
	lcd = initialize_lcd();
	
	
	//Init interrupts on the pins
	wiringPiISR(BUTTON_SELECT, INT_EDGE_FALLING, *wait_command);
	wiringPiISR(BUTTON_UP, INT_EDGE_FALLING, *increase);
	wiringPiISR(BUTTON_DOWN, INT_EDGE_FALLING, *decrease);	
	
	while(state != Exit){
		//Loops and show data when in the menu state
		if (state == Menu){
			int i;
			sensors *sensor = NULL;
			sensor = (sensors *) malloc(sizeof(sensors));
			time_t rawtime;
			struct tm * timeinfo;
		
			time(&rawtime);
			timeinfo = localtime(&rawtime);
		
			data = read_dht11_dat();
			data_i2c = i2c_comns(data_i2c);
			sensor->lum = convert_data(data_i2c->sensor_1, 40);
			sensor->press = convert_data(data_i2c->sensor_2, 10);	
			sensor->temp = data->temp;
			sensor->hum = data->hum;
			sensor->time_stamp = asctime(timeinfo);	
			
			
			if (hist_size < 10){
				history[hist_size++] = sensor;
			}else{
				free(history[9]);
				for (i=8; i>=0; i--){
					history[i+1] = history[i];
				}
				history[0] = sensor;
			}
			send_data(mosq);
			lcdHome(lcd);
			lcdClear(lcd);
			printSensorData(lcd, sensor);	
			sleep(interval);
		}
	}	
	
	
	int i;
	for(i=0; i<10; i++) free(history[i]);
	free(data_i2c);
	free(data);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
	return 0;
}

double convert_data(double value, int factor){
	return (value*factor)/25500;
}


i2c_data *i2c_comns(){
	int I2CFile;
	int addr = 0x48;
 	uint8_t write_buffer[3];
 	uint8_t read_buffer[2]; 	
 	
	uint16_t value;
	
	i2c_data *data = NULL;
	data = (i2c_data *) malloc(sizeof(i2c_data));
	
	//Opens the i2c port 
	I2CFile = open("/dev/i2c-1", O_RDWR);	
	ioctl(I2CFile, I2C_SLAVE, addr);
	
	write_buffer[0] = 1;
	write_buffer[1] = 0xC3;
	write_buffer[2] = 0x03;
	
	read_buffer[0] = 0;
	read_buffer[1] = 0;
	
	write(I2CFile, write_buffer, 3);
	
	//Reads the first two byte data
	while((read_buffer[0] & 0x80) == 0){
		read(I2CFile, read_buffer, 2);
	}
	
	write_buffer[0] = 0;
	write(I2CFile, write_buffer, 1);
	
	read(I2CFile, read_buffer, 2);
	
	value = (read_buffer[0] << 8) | read_buffer[1];
	
	data->sensor_1 = value;
	
	write_buffer[0] = 1;
	write_buffer[1] = 0xF3;
	write_buffer[2] = 0x03;
	
	read_buffer[0] = 0;
	read_buffer[1] = 0;
	
	write(I2CFile, write_buffer, 3);
	//Reads the sencond two byte data
	while((read_buffer[0] & 0x80) == 0){
		read(I2CFile, read_buffer, 2);
	}
	
	write_buffer[0] = 0;
	write(I2CFile, write_buffer, 1);
	
	read(I2CFile, read_buffer, 2);
	
	value = (read_buffer[0] << 8) | read_buffer[1];
	data->sensor_2 = value;
	close(I2CFile);
	return data;
}




int initialize_lcd(){
	//Initialize LCD pinout
	int lcdHandler;
	lcdHandler = lcdInit(2, 16, 4, 6, 31, 26, 27, 28, 29, 0, 0, 0, 0);
	if(lcdHandler == -1) return -1;
	lcdDisplay(lcdHandler, 1);
	return lcdHandler;

}


void printSensorData(int lcdHandler, sensors *sensor_data){
	//prints all sensor data onto the display
	lcdHome(lcdHandler);
	lcdClear(lcdHandler);
	char *message = "%0.1fC | %0.1fH";
	lcdPrintf(lcdHandler, message, sensor_data->temp, sensor_data->hum);
	lcdPosition(lcdHandler, 0, 1);	
	message = "%0.1fkL | %0.1fmb";
	lcdPrintf(lcdHandler, message, sensor_data->lum, sensor_data->press);
	
}


dht11 *read_dht11_dat(){
	uint8_t laststate	= HIGH;
	uint8_t counter		= 0;
	uint8_t j		= 0, i;
	unsigned int dht11_dat[5];
	dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;

	// pull pin down for 18 milliseconds 
	pinMode( DHTPIN, OUTPUT );
	digitalWrite( DHTPIN, LOW );
	delay( 18 );
	// then pull it up for 40 microseconds 
	digitalWrite( DHTPIN, HIGH );
	delayMicroseconds( 20 );
	// prepare to read the pin 
	pinMode( DHTPIN, INPUT );

	// detect change and read data 
	for ( i = 0; i < MAXTIMINGS; i++ ){
		counter = 0;
		while ( digitalRead( DHTPIN ) == laststate ){
			counter++;
			delayMicroseconds( 1 );
			if ( counter == 255 ){
				break;
			}
		}
		laststate = digitalRead( DHTPIN );

		if ( counter == 255 ){
			break;
		}
		
		// ignore first 3 transitions 
		if ( (i >= 4) && (i % 2 == 0) ){
			// shove each bit into the storage bytes 
			dht11_dat[j / 8] <<= 1;
			if ( counter > 16 )
				dht11_dat[j / 8] |= 1;
			j++;
		}
	}
	
	 
	dht11 *data = NULL;
	
	data = (dht11 *) malloc(sizeof(dht11));
	char temp[10];
	char hum[10];
	char *dummy;
	
	snprintf(temp, 10, "%d.%d", dht11_dat[2], dht11_dat[3]);
	snprintf(hum, 10, "%d.%d",  dht11_dat[0], dht11_dat[1]);
	
	data->temp = strtod(temp, &dummy);
	data->hum = strtod(hum, &dummy);
	
	return data;
}

void wait_command(){
	pinMode(LEVER_MENU, INPUT);
	pinMode(LEVER_EXIT, INPUT);
	pinMode(LEVER_HISTORY, INPUT);
	pinMode(LEVER_TIME, INPUT);
	//Creates Menu Options attached to Button
	if(digitalRead(LEVER_MENU) == HIGH && digitalRead(LEVER_EXIT) == LOW && digitalRead(LEVER_HISTORY) == LOW && digitalRead(LEVER_TIME) == LOW){
		state = Menu;
	}else if(digitalRead(LEVER_MENU) == LOW && digitalRead(LEVER_EXIT) == HIGH && digitalRead(LEVER_HISTORY) == LOW && digitalRead(LEVER_TIME) == LOW){
		state = Exit;
	}else if(digitalRead(LEVER_MENU) == LOW && digitalRead(LEVER_EXIT) == LOW && digitalRead(LEVER_HISTORY) == HIGH && digitalRead(LEVER_TIME) == LOW){
		state = History;
		lcdHome(lcd);
		lcdClear(lcd);
		lcdPrintf(lcd, "%s", history[hist_posi]->time_stamp);
		sleep(2);
		lcdHome(lcd);
		lcdClear(lcd);
		printSensorData(lcd, history[hist_posi]);
	}else if(digitalRead(LEVER_MENU) == LOW && digitalRead(LEVER_EXIT) == LOW && digitalRead(LEVER_HISTORY) == LOW && digitalRead(LEVER_TIME) == HIGH){
		state = Time;
		lcdHome(lcd);
		lcdClear(lcd);
		lcdPrintf(lcd, "Interval: %d", interval);
	}else{
		lcdHome(lcd);
		lcdClear(lcd);
		lcdPrintf(lcd, "Option Unavailable!");
	}
}


void increase(){
	lcdHome(lcd);
	lcdClear(lcd);
	if(state == Time){
		lcdPrintf(lcd, "Interval: %d", ++interval);
	}else if (state == History){
		if((hist_posi + 1) >= hist_size){
			lcdPrintf(lcd, "Invalid Interval!");
		}else{
			hist_posi++;
			lcdPrintf(lcd, "%s", history[hist_posi]->time_stamp);
			sleep(2);
			lcdHome(lcd);
			lcdClear(lcd);
			printSensorData(lcd, history[hist_posi]);
		}
	}
}


void decrease(){
	lcdHome(lcd);
	lcdClear(lcd);
	if(state == Time){
		if((interval - 1) <= 0)
			lcdPrintf(lcd, "Invalid Interval!");
		else
			lcdPrintf(lcd, "Interval: %d", --interval);
	}else if (state == History){
		if((hist_posi - 1) < 0 ){
			lcdPrintf(lcd, "Invalid Interval!");
		}else{
			hist_posi--;
			lcdPrintf(lcd, "%s\n", history[hist_posi]->time_stamp);
			sleep(2);
			lcdHome(lcd);
			lcdClear(lcd);
			printSensorData(lcd, history[hist_posi]);
		}
	}
}

void send_data (struct mosquitto *mosq) {
		//Publishes Humidity data
		char value[10];
    	snprintf(value, 10, "%f", history[0]->hum);
        char str1[100] = "{\"status\":1, \"value\": ";
        char str2[100] = ", \"tempo\": \"";
        strcat(str1, value);
        strcat(str1, str2);
        strcat(str1, history[0]->time_stamp);
        strcat(str1, "\"}");
        char * umidade = str1;
        mosquitto_publish(mosq,NULL,"sensor/umidade", strlen(umidade), umidade, 0, 0);

		//Publishes Temperature data
    	snprintf(value, 10, "%f", history[0]->temp);
        strcpy(str1, "{\"status\":1, \"value\": ");
        strcpy(str2, ", \"tempo\": \"");
        strcat(str1, value);
        strcat(str1, str2);
        strcat(str1, history[0]->time_stamp);
        strcat(str1, "\"}");
        char * temperatura = str1;
        mosquitto_publish(mosq,NULL,"sensor/temperatura", strlen(temperatura), temperatura,0,0);

		//Publishes Pressure data
    	snprintf(value, 10, "%f", history[0]->press);
        strcpy(str1, "{\"status\":1, \"value\": ");
        strcpy(str2, ", \"tempo\": \"");
        strcat(str1, value);
        strcat(str1, str2);
        strcat(str1, history[0]->time_stamp);
        strcat(str1, "\"}");
        char * pressao = str1;
        mosquitto_publish(mosq,NULL,"sensor/pressao", strlen(pressao), pressao,0,0);


		//Publishes Luminosity data
    	snprintf(value, 10, "%f", history[0]->lum);
        strcpy(str1, "{\"status\":1, \"value\": ");
        strcpy(str2, ", \"tempo\": \"");
        strcat(str1, value);
        strcat(str1, str2);
        strcat(str1, history[0]->time_stamp);
        strcat(str1, "\"}");
        char * luminosidade = str1;
        mosquitto_publish(mosq,NULL,"sensor/luminosidade", strlen(luminosidade), luminosidade, 0, 0);
}

void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
	//Responds a request from mobile
    if(message->payloadlen && history[0]!=NULL) {
        char * mensagem = (char *)message->payload;
        int temp = strcmp("sensor/tempo/set",message->topic);

    	char value[10];
    	snprintf(value, 10, "%f", history[0]->hum);
        char str1[100] = "{\"status\":1, \"value\": ";
        char str2[100] = ", \"tempo\": \"";
        strcat(str1, value);
        strcat(str1, str2);
        strcat(str1, history[0]->time_stamp);
        strcat(str1, "\"}");
        char * umidade = str1;
        mosquitto_publish(mosq,NULL,"sensor/umidade", strlen(umidade), umidade, 0, 0);
        printf("--- %s %s", message->topic, mensagem);


    	snprintf(value, 10, "%f", history[0]->temp);
        strcpy(str1, "{\"status\":1, \"value\": ");
        strcpy(str2, ", \"tempo\": \"");
        strcat(str1, value);
        strcat(str1, str2);
        strcat(str1, history[0]->time_stamp);
        strcat(str1, "\"}");
        char * temperatura = str1;
        mosquitto_publish(mosq,NULL,"sensor/temperatura", strlen(temperatura), temperatura,0,0);
        printf("--- %s %s", message->topic, mensagem);


    	snprintf(value, 10, "%f", history[0]->press);
        strcpy(str1, "{\"status\":1, \"value\": ");
        strcpy(str2, ", \"tempo\": \"");
        strcat(str1, value);
        strcat(str1, str2);
        strcat(str1, history[0]->time_stamp);
        strcat(str1, "\"}");
        char * pressao = str1;
        mosquitto_publish(mosq,NULL,"sensor/pressao", strlen(pressao), pressao,0,0);
        printf("--- %s %s", message->topic, mensagem);

    	snprintf(value, 10, "%f", history[0]->lum);
        strcpy(str1, "{\"status\":1, \"value\": ");
        strcpy(str2, ", \"tempo\": \"");
        strcat(str1, value);
        strcat(str1, str2);
        strcat(str1, history[0]->time_stamp);
        strcat(str1, "\"}");
        char * luminosidade = str1;
        mosquitto_publish(mosq,NULL,"sensor/luminosidade", strlen(luminosidade), luminosidade, 0, 0);
        printf("--- %s %s", message->topic, mensagem);


		//Changes the Work Interval from information retrieval
        if (temp == 0) {
            int mTemp = atoi(mensagem);
			//Changes interval
            interval = mTemp;
            printf("%i", mTemp);
            char * msg = "{\"status\":1}";
            mosquitto_publish(mosq,NULL,"sensor/tempo", strlen(msg), msg, 0, 0);

        } 
    }else{
        printf("%s (null)\n", message->topic);
    }
    fflush(stdout);
}

void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
    int i;
    if(!result){
        /* Subscribe to broker information topics on successful connect. */
        mosquitto_subscribe(mosq, NULL, "sensor/umidade/get", 0);
        mosquitto_subscribe(mosq, NULL, "sensor/temperatura/get", 0);
        mosquitto_subscribe(mosq, NULL, "sensor/pressao/get", 0);
        mosquitto_subscribe(mosq, NULL, "sensor/luminosidade/get", 0);
        mosquitto_subscribe(mosq, NULL, "sensor/tempo/set", 0);
        mosquitto_subscribe(mosq, NULL, "sensor/historico/get", 0);
    }else{
        fprintf(stderr, "Connect failed\n");
    }
}

void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
    int i;
    printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
    for(i=1; i<qos_count; i++){
        printf(", %d", granted_qos[i]);
    }
    printf("\n");
}

void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
    printf("%s\n", str);
}
