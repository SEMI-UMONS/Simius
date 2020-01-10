/*


*/
unsigned long sampletime_ms = 30000;				//sample 30s 
float ratio = 0;
float concentration = 0;
char AqiString[11] = "";
int AQI= 0;
int aqiPM10=0;
//------------------------xxx---------------------

/*
 * AQI formula: https://en.wikipedia.org/wiki/Air_Quality_Index#United_States
 * Arduino code https://gist.github.com/nfjinjing/8d63012c18feea3ed04e
 * On line AQI calculator https://www.airnow.gov/index.cfm?action=resources.conc_aqi_calc
 */
float calcAQI(float I_high, float I_low, float C_high, float C_low, float C) {
  return (I_high - I_low) * (C - C_low) / (C_high - C_low) + I_low;
}

int getAQI(int sensor, float density) {
  int d10 = (int)(density * 10);
  if ( sensor == 0 ) {
    if (d10 <= 0) {
      return 0;
    }
    else if(d10 <= 120) {
      return calcAQI(50, 0, 120, 0, d10);
    }
    else if (d10 <= 354) {
      return calcAQI(100, 51, 354, 121, d10);
    }
    else if (d10 <= 554) {
      return calcAQI(150, 101, 554, 355, d10);
    }
    else if (d10 <= 1504) {
      return calcAQI(200, 151, 1504, 555, d10);
    }
    else if (d10 <= 2504) {
      return calcAQI(300, 201, 2504, 1505, d10);
    }
    else if (d10 <= 3504) {
      return calcAQI(400, 301, 3504, 2505, d10);
    }
    else if (d10 <= 5004) {
      return calcAQI(500, 401, 5004, 3505, d10);
    }
    else if (d10 <= 10000) {
      return calcAQI(1000, 501, 10000, 5005, d10);
    }
    else {
      return 1001;
    }
  } else {
    if (d10 <= 0) {
      return 0;
    }
    else if(d10 <= 540) {
      return calcAQI(50, 0, 540, 0, d10);
    }
    else if (d10 <= 1540) {
      return calcAQI(100, 51, 1540, 541, d10);
    }
    else if (d10 <= 2540) {
      return calcAQI(150, 101, 2540, 1541, d10);
    }
    else if (d10 <= 3550) {
      return calcAQI(200, 151, 3550, 2541, d10);
    }
    else if (d10 <= 4250) {
      return calcAQI(300, 201, 4250, 3551, d10);
    }
    else if (d10 <= 5050) {
      return calcAQI(400, 301, 5050, 4251, d10);
    }
    else if (d10 <= 6050) {
      return calcAQI(500, 401, 6050, 5051, d10);
    }
    else {
      return 1001;
    }
  }   
}

/*
 * CAQI Européen - European CAQI level 
 * source : http://www.airqualitynow.eu/about_indices_definition.php
 */
int getACQI( int sensor, float density ){  
  if ( sensor == 0 ) {  //PM2,5
    if ( density == 0 ) {
      return 0; 
    } else if ( density <= 15 ) {
      return 25 ;
    } else if ( density > 15 && density <= 30 ) {
      return 50;
    } else if ( density > 30 && density <= 55 ) {
      return 75;
    } else if ( density > 55 && density <= 110 ) {
      return 100;
    } else {
      return 150;
    }
  } else {              //PM10
    if ( density == 0 ) {
      return 0; 
    } else if ( density <= 25 ) {
      return 25 ;
    } else if ( density > 25 && density <= 50 ) {
      return 50;
    } else if ( density > 50 && density <= 90 ) {
      return 75;
    } else if ( density > 90 && density <= 180 ) {
      return 100;
    } else {
      return 150;
    }
  }
}

/*
 * Calcul l'indice de qualité de l'air français ATMO
 * Calculate French ATMO AQI indicator
 */
int getATMO( int sensor, float density ){
  if ( sensor == 0 ) { //PM2,5
    if ( density <= 11 ) {
      return 1; 
    } else if ( density > 11 && density <= 24 ) {
      return 2;
    } else if ( density > 24 && density <= 36 ) {
      return 3;
    } else if ( density > 36 && density <= 41 ) {
      return 4;
    } else if ( density > 41 && density <= 47 ) {
      return 5;
    } else if ( density > 47 && density <= 53 ) {
      return 6;
    } else if ( density > 53 && density <= 58 ) {
      return 7;
    } else if ( density > 58 && density <= 64 ) {
      return 8;
    } else if ( density > 64 && density <= 69 ) {
      return 9;
    } else {
      return 10;
    }
  } else {
    if ( density <= 6 ) {
      return 1; 
    } else if ( density > 6 && density <= 13 ) {
      return 2;
    } else if ( density > 13 && density <= 20 ) {
      return 3;
    } else if ( density > 20 && density <= 27 ) {
      return 4;
    } else if ( density > 27 && density <= 34 ) {
      return 5;
    } else if ( density > 34 && density <= 41 ) {
      return 6;
    } else if ( density > 41 && density <= 49 ) {
      return 7;
    } else if ( density > 49 && density <= 64 ) {
      return 8;
    } else if ( density > 64 && density <= 79 ) {
      return 9;
    } else {
      return 10;
    }  
  }
}
void updateAQIDisplay(byte country){
  /*
   * 1 EXCELLENT                    
   * 2 GOOD                         
   * 3 ACCEPTABLE               
   * 4 MODERATE            
   * 5 HEAVY               
   * 6 SEVERE
   * 7 HAZARDOUS
   */
  if ( COUNTRY == 0 ) {
    // Système ATMO français - French ATMO AQI system 
    switch (AQI) {
      case 10: 
        AqiString = "SEVERE";
        break;
      case 9:
        AqiString = "HEAVY";
        break;
      case 8:
        AqiString = "HEAVY";
        break;  
      case 7:
        AqiString = "MODERATE";
        break;
      case 6:
        AqiString = "MODERATE";
        break;   
      case 5:
        AqiString = "ACCEPTABLE";
        break;
      case 4:
        AqiString = "GOOD";
        break;
      case 3:
        AqiString = "GOOD";
        break;
      case 2:
        AqiString = "EXCELLENT";
        break;
      case 1:
        AqiString = "EXCELLENT";
        break;           
      }
  } else if ( COUNTRY == 1 ) {
    // European CAQI
    switch (AQI) {
      case 25: 
        AqiString = "GOOD";
        break;
      case 50:
        AqiString = "ACCEPTABLE";
        break;
      case 75:
        AqiString = "MODERATE";
        break;
      case 100:
        AqiString = "HEAVY";
        break;         
      default:
        AqiString = "SEVERE";
      }  
  } 
}