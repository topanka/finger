//#include "Time.h"
#include "fim5360.h"

char g_in_cmd[50]={0};
char *g_argv=NULL;
unsigned long g_in_cmdlen=0;
unsigned long g_millis;
unsigned long g_do_time;
unsigned long g_hold_time;
unsigned long g_sop_time;
int g_do_port;
unsigned long g_op_time;
int g_op_succ=0;
int g_op_fail=0;
int g_op_completed=0;
int g_autoidon=0;
int g_autoidactive=0;
int g_registerph=0;
int g_outtype=0;
FIM5360SPH g_fim5360={0};
FIM5360SPH g_fim5360r={0};
unsigned char g_recvbuf_d[512];

void swap(uint32_t *x)
{
*x = (((*x)>>24)&0xff) | // move byte 3 to byte 0
      (((*x)<<8)&0xff0000) | // move byte 1 to byte 2
      (((*x)>>8)&0xff00) | // move byte 2 to byte 1
      (((*x)<<24)&0xff000000); // byte 0 to byte 3
}

void buildcmd_body(FIM5360SPH *sph)
{
  int i;
  
  sph->u.hcs.chsum=sph->u.bufx[0];
  for(i=1;i < sizeof(FIM5360HDT);i++) {
    sph->u.hcs.chsum+=sph->u.bufx[i];
  }
  swap(&sph->u.hcs.chsum);

  swap(&sph->u.hcs.hdt.cmd);
  swap(&sph->u.hcs.hdt.p1);
  swap(&sph->u.hcs.hdt.p2);
  swap(&sph->u.hcs.hdt.ds);
  
  sph->sdr=0;
}

int buildcmd_reqconn(FIM5360SPH *sph)
{
  sph->u.hcs.hdt.cmd=CMD_REQUEST_CONNECTION;
  sph->u.hcs.hdt.p1=0;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=0;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_fmver(FIM5360SPH *sph)
{
  sph->u.hcs.hdt.cmd=CMD_GET_FIRMWARE_VERSION2;
  sph->u.hcs.hdt.p1=0;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=0;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_devinfo(FIM5360SPH *sph)
{
  sph->u.hcs.hdt.cmd=CMD_GET_DEVICE_INFO;
  sph->u.hcs.hdt.p1=0;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=0;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_identifyfp(FIM5360SPH *sph)
{
  sph->u.hcs.hdt.cmd=CMD_IDENTIFY_FP;
  sph->u.hcs.hdt.p1=0;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=0;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_verifyfp(FIM5360SPH *sph, unsigned long dl)
{
  sph->u.hcs.hdt.cmd=CMD_VERIIFY_FP;
  sph->u.hcs.hdt.p1=0;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=dl;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_entermm(FIM5360SPH *sph, unsigned long mm, unsigned long dl)
{
  sph->u.hcs.hdt.cmd=CMD_ENTER_MASTER_MODE2;
  sph->u.hcs.hdt.p1=mm;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=dl;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_leavemm(FIM5360SPH *sph)
{
  sph->u.hcs.hdt.cmd=CMD_LEAVE_MASTER_MODE;
  sph->u.hcs.hdt.p1=0;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=0;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_getfplist(FIM5360SPH *sph, unsigned long lt)
{
  sph->u.hcs.hdt.cmd=CMD_GET_FP_LIST2;
  sph->u.hcs.hdt.p1=lt;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=0;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_getmalist(FIM5360SPH *sph, unsigned long lt)
{
  sph->u.hcs.hdt.cmd=CMD_GET_MASTER_LIST2;
  sph->u.hcs.hdt.p1=lt;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=0;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_setmaster(FIM5360SPH *sph, unsigned long setflg, unsigned long dl)
{
  sph->u.hcs.hdt.cmd=CMD_SET_MASTER;
  sph->u.hcs.hdt.p1=setflg;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=dl;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_setbpwd(FIM5360SPH *sph, unsigned long dl)
{
  sph->u.hcs.hdt.cmd=CMD_SET_MASTER_PASSWORD;
  sph->u.hcs.hdt.p1=0;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=dl;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_getlogd(FIM5360SPH *sph, unsigned long p1, unsigned long p2)
{
  sph->u.hcs.hdt.cmd=CMD_READ_LOG_DATA;
  sph->u.hcs.hdt.p1=p1;
  sph->u.hcs.hdt.p2=p2;
  sph->u.hcs.hdt.ds=0;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_logging(FIM5360SPH *sph, unsigned long enable)
{
  sph->u.hcs.hdt.cmd=CMD_SET_LOG_OPTION;
  sph->u.hcs.hdt.p1=0;
  sph->u.hcs.hdt.p2=enable;
  sph->u.hcs.hdt.ds=0;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_getlogopt(FIM5360SPH *sph)
{
  sph->u.hcs.hdt.cmd=CMD_GET_LOG_OPTION;
  sph->u.hcs.hdt.p1=0;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=0;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_getsysi(FIM5360SPH *sph, unsigned long sitype)
{
  sph->u.hcs.hdt.cmd=CMD_GET_SYS_INFO;
  sph->u.hcs.hdt.p1=sitype;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=0;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_autoid(FIM5360SPH *sph, unsigned long on)
{
  sph->u.hcs.hdt.cmd=CMD_AUTO_IDENTIFY;
  sph->u.hcs.hdt.p1=on;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=0;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_deletefp(FIM5360SPH *sph, unsigned long dl)
{
  sph->u.hcs.hdt.cmd=CMD_DELETE_FP;
  sph->u.hcs.hdt.p1=0;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=dl;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_deleteallfp(FIM5360SPH *sph, uint32_t optype)
{
  sph->u.hcs.hdt.cmd=CMD_DELETE_ALL_FP;
  sph->u.hcs.hdt.p1=optype;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=0;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_register(FIM5360SPH *sph, unsigned long master, unsigned long mode, unsigned long dl)
{
  sph->u.hcs.hdt.cmd=CMD_REGISTER_FP;
  sph->u.hcs.hdt.p1=master;
  sph->u.hcs.hdt.p2=mode;
  sph->u.hcs.hdt.ds=dl;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_getimagequality(FIM5360SPH *sph)
{
  sph->u.hcs.hdt.cmd=CMD_GET_IMAGE_QUALITY;
  sph->u.hcs.hdt.p1=0;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=0;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_writeuserdata(FIM5360SPH *sph, unsigned long addr, unsigned long dl)
{
  sph->u.hcs.hdt.cmd=CMD_WRITE_USER_DATA;
  sph->u.hcs.hdt.p1=addr;
  sph->u.hcs.hdt.p2=dl;
  sph->u.hcs.hdt.ds=dl;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_readuserdata(FIM5360SPH *sph, unsigned long addr, unsigned long dl)
{
  sph->u.hcs.hdt.cmd=CMD_READ_USER_DATA;
  sph->u.hcs.hdt.p1=addr;
  sph->u.hcs.hdt.p2=dl;
  sph->u.hcs.hdt.ds=0;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_delmapwd(FIM5360SPH *sph)
{
  sph->u.hcs.hdt.cmd=CMD_DELETE_MASTER_PASSWORD;
  sph->u.hcs.hdt.p1=0;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=0;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_gettime(FIM5360SPH *sph)
{
  sph->u.hcs.hdt.cmd=CMD_GET_TIME;
  sph->u.hcs.hdt.p1=0;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=0;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int buildcmd_settime(FIM5360SPH *sph)
{
  sph->u.hcs.hdt.cmd=CMD_SET_TIME;
  sph->u.hcs.hdt.p1=0;
  sph->u.hcs.hdt.p2=0;
  sph->u.hcs.hdt.ds=8;
  sph->u.hcs.hdt.ec=0;

  buildcmd_body(sph);
  
  return(0);
}

int fim5360_send(FIM5360SPH *sph, const unsigned char *dd)
{
  int i;
  uint32_t chsum;
  unsigned long dl=0;

  while(Serial1.available()) {
    Serial1.read();
  }
  
  if(dd != NULL) {
    dl=sph->u.hcs.hdt.ds;
    swap(&dl);
    if(dl == 0) return(-1);
    chsum=*(dd+0);
    for(i=1;i < dl;i++) {
      chsum+=*(dd+i);
    }
    swap(&chsum);
  }
  Serial1.write(0x7E);
  Serial1.write(sph->u.bufx,sizeof(sph->u.bufx));

Serial.print("Data sent: ");
Serial.print(0x7E,HEX);
for(i=0;i < sizeof(sph->u.bufx);i++) {
  Serial.print(sph->u.bufx[i],HEX);
}
Serial.println("");
  
  if(dd != NULL) {
Serial.print("Sending data and chsum: ");
Serial.println(dl);
    Serial1.write(dd,dl);
    Serial1.write((byte*)&chsum,sizeof(chsum));
  }
  swap(&sph->u.hcs.hdt.cmd);
  return(0);
}

/*
unsigned char serial_read_timed(int tmo)
{
  while(
}
*/

int fim5360_checkhchsum(FIM5360SPH *sph)
{
  int i;
  uint32_t chsum;
  
  chsum=sph->u.bufx[0];
  for(i=1;i < sizeof(FIM5360HDT);i++) {
    chsum+=sph->u.bufx[i];
  }
  swap(&sph->u.hcs.chsum);
/*  
  Serial.print("Chsum on read: ");
  Serial.print(sph->u.hcs.chsum);
  Serial.print(" ");
  Serial.println(chsum);
*/  
  if(sph->u.hcs.chsum == chsum) return(1);
  return(0);
}

int fim5360_checkdchsum(FIM5360SPH *sph, unsigned char *dd)
{
  int i;
  uint32_t chsum1;
  uint32_t chsum2;
  
  chsum1=*(dd+0);
  for(i=1;i < sph->u.hcs.hdt.ds;i++) {
    chsum1+=(*(dd+i));
  }
  memcpy((void*)&chsum2,(void*)(dd+sph->u.hcs.hdt.ds),4);
  swap(&chsum2);
/*  
  Serial.print("DATA chsum: ");
  Serial.print(chsum1);
  Serial.print(" ");
  Serial.println(chsum2);
*/  
  return(0);
}

int fim5360_read(FIM5360SPH *sph, unsigned char *dd, unsigned long dl)
{
  int i,din0n=0;
  uint32_t chsum,l;
  unsigned char c1;

//  Serial.println("fim5360_read0");

  while(Serial1.available()) {
    
//  Serial.println("fim5360_read1");

    c1=Serial1.read();
    if((sph->sdr == 0) && (sph->lead != 0x7E)) {
      if(c1 != 0x7E) {
        Serial.println("no lead byte first");
        while(Serial1.available()) {
          c1=Serial1.read();
          if(c1 == 0x7E) break;
        }
        if(c1 != 0x7E) {
          return(-1);
        }
      }
    }
    if((sph->lead != 0x7E) && (c1 == 0x7E)) {
      sph->lead=0x7E;
      sph->u.hcs.hdt.ds=0;
      continue;
    }
    if(sph->sdr >= sizeof(FIM5360HCS)) {
      return(-2);
    }
    sph->u.bufx[sph->sdr]=c1;
    sph->sdr++;
    if(sph->sdr == sizeof(FIM5360HCS)) {
      if(fim5360_checkhchsum(sph) != 1) return(-3);
      if(sph->u.hcs.hdt.ds != 0) {
        swap(&sph->u.hcs.hdt.ds);
        l=0;
        while(l < sph->u.hcs.hdt.ds+4) {
          din0n++;
          while(Serial1.available()) {
            din0n=0;
            c1=Serial1.read();
            if((dd != NULL) && (l < dl)) {
              *(dd+l)=c1;
              l++;
            }
          }
          if(din0n > 100) {
            Serial.print(sph->u.hcs.hdt.ds);
            Serial.print(" ");
            Serial.println(l);
            return(-4);
          }
          delay(1);
        }
        fim5360_checkdchsum(sph,dd);
      }
      return(1);
    }
  }  
  return(0);
}

int readcmd(void)
{
  static unsigned long l_cmdin=0;
  
  if(l_cmdin == 0) {
    g_argv=NULL;
    if(Serial.available()) {
      l_cmdin=g_millis;
      while(Serial.available()) {
        g_in_cmd[g_in_cmdlen++]=Serial.read();
        if(g_in_cmdlen >= sizeof(g_in_cmd)) {
          g_in_cmd[0]='\0';
          g_in_cmdlen=0;
          return(-1);
        }
      }
    }
  } else {
    while(Serial.available()) {
      g_in_cmd[g_in_cmdlen++]=Serial.read();
      if(g_in_cmdlen >= sizeof(g_in_cmd)) {
        g_in_cmd[0]='\0';
        g_in_cmdlen=0;
        return(-1);
      }
    }
    if((l_cmdin+15) < g_millis) {
      g_in_cmd[g_in_cmdlen]='\0';
      if((g_argv=strchr(g_in_cmd,' ')) != NULL) {
        *g_argv='\0';
        g_argv++;
      }
      l_cmdin=0;
      return(1);
    }
  }
  return(0);
}

/*
void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.println();
}
*/

void setup() {
 
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(9600);
  pinMode(FIM5360_SUCC_PORT,INPUT_PULLUP);
  pinMode(FIM5360_FAIL_PORT,INPUT_PULLUP);
  pinMode(FIM5360_REG_PORT,OUTPUT);
  pinMode(FIM5360_DEL_PORT,OUTPUT);
  pinMode(FIM5360_IDENT_PORT,OUTPUT);
  digitalWrite(FIM5360_REG_PORT,HIGH);
  digitalWrite(FIM5360_DEL_PORT,HIGH);
  digitalWrite(FIM5360_IDENT_PORT,HIGH);
  g_do_time=0;
  g_do_port=0;
  g_op_time=0;
  g_op_completed=1;
  g_hold_time=50;
  Serial.println("Initialized");
}

byte decToBcd(byte val)
{
  return( ( (val/10)*16) + (val%10) );
}

int getbcdtime(char *tstr, unsigned char *tbcd)
{
  int i;
  char *p,n1[3]={0};
  
  p=tstr;

  for(i=0;i < 7;i++) {
    tbcd[i]=(*p++-48)*16;
    tbcd[i]+=(*p++-48);
  }
  tbcd[7]=0;

  Serial.print("bcd date: ");
  Serial.print(tstr);
  Serial.print(" ");
  for(i=0;i < 4;i++) {
    Serial.print(tbcd[2*i],HEX);
    Serial.print(tbcd[2*i+1],HEX);
  }
  Serial.println("");
  
  return(0);
}

void loop() {
  int ret,i,rcmd;
  const unsigned char *ptr;
  unsigned long ec,res;
  unsigned char tmbuf[8];
  
  // put your main code here, to run repeatedly:
  g_millis=millis();

  rcmd=readcmd();
  
  if(rcmd == 1) {
    
//    digitalClockDisplay();
    if(g_argv != NULL) {
      Serial.println(g_argv);
    }
    
    g_outtype=0;
    if(strcmp(g_in_cmd,"reqconn") == 0) {
        g_op_completed=0;
        buildcmd_reqconn(&g_fim5360);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_REQUEST_CONNNECTION on progress ...");
    } else if(strcmp(g_in_cmd,"fmver") == 0) {
        g_op_completed=0;
        buildcmd_fmver(&g_fim5360);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_GET_FIRMWARE_VERSION2 on progress ...");
    } else if(strcmp(g_in_cmd,"devinfo") == 0) {
        g_op_completed=0;
        buildcmd_devinfo(&g_fim5360);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_GET_DEVICE_INFO on progress ...");
    } else if(strcmp(g_in_cmd,"identifyfp") == 0) {
        g_op_completed=0;
        buildcmd_identifyfp(&g_fim5360);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_IDENTIFY_FP on progress ...");
    } else if(strcmp(g_in_cmd,"verifyfp") == 0) {
//        ptr=(const unsigned char*)"0001\0\0\0\0\0\0\0\0\0\0";
//        ptr=(const unsigned char*)"0000\0\0\0\0\0\0\0\0\0\0";
        ptr=(const unsigned char*)"0006\0\0\0\0\0\0\0\0\0\0";
        g_op_completed=0;
        buildcmd_verifyfp(&g_fim5360,10);
        fim5360_send(&g_fim5360,ptr);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_VERIFY_FP on progress ...");
    } else if(strcmp(g_in_cmd,"entermm0") == 0) {
        g_op_completed=0;
        buildcmd_entermm(&g_fim5360,3,0);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_ENTER_MASTER_MODE2 on progress ...");
    } else if(strcmp(g_in_cmd,"entermmfp") == 0) {
        g_op_completed=0;
        ptr=(const unsigned char*)"0001\0\0\0\0\0\0\0\0\0\0";
        buildcmd_entermm(&g_fim5360,0,10);
        fim5360_send(&g_fim5360,ptr);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_ENTER_MASTER_MODE2 on progress ...");
    } else if(strcmp(g_in_cmd,"entermmbpwd") == 0) {
        g_op_completed=0;
        ptr=(const unsigned char*)"aA123456\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
        buildcmd_entermm(&g_fim5360,2,16);
        fim5360_send(&g_fim5360,ptr);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_ENTER_MASTER_MODE2 on progress ...");
    } else if(strcmp(g_in_cmd,"leavemm") == 0) {
        g_op_completed=0;
        buildcmd_leavemm(&g_fim5360);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_LEAVE_MASTER_MODE2 on progress ...");
    } else if(strcmp(g_in_cmd,"getfplist") == 0) {
        g_op_completed=0;
        buildcmd_getfplist(&g_fim5360,0);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_GET_FP_LIST2 on progress ...");
    } else if(strcmp(g_in_cmd,"getmalist") == 0) {
        g_op_completed=0;
        buildcmd_getmalist(&g_fim5360,0);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_GET_MASTER_LIST2 on progress ...");
    } else if(strcmp(g_in_cmd,"setmaster") == 0) {
        ptr=(const unsigned char*)"0006\0\0\0\0\0\0\0\0\0\0";
//        ptr=(const unsigned char*)"0000\0\0\0\0\0\0\0\0\0\0";
        g_op_completed=0;
        buildcmd_setmaster(&g_fim5360,1,10);
        fim5360_send(&g_fim5360,ptr);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_SET_MASTER on progress ...");
    } else if(strcmp(g_in_cmd,"clrmaster") == 0) {
        ptr=(const unsigned char*)"0001\0\0\0\0\0\0\0\0\0\0";
//        ptr=(const unsigned char*)"0000\0\0\0\0\0\0\0\0\0\0";
        g_op_completed=0;
        buildcmd_setmaster(&g_fim5360,0,10);
        fim5360_send(&g_fim5360,ptr);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_SET_MASTER on progress ...");
    } else if(strcmp(g_in_cmd,"setbpwd") == 0) {
        ptr=(const unsigned char*)"aA123456\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
        g_op_completed=0;
        buildcmd_setbpwd(&g_fim5360,16);
        fim5360_send(&g_fim5360,ptr);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_SET_MASTER_PASSWORD on progress ...");
    } else if(strcmp(g_in_cmd,"getlogd") == 0) {
        g_op_completed=0;
        buildcmd_getlogd(&g_fim5360,2,0);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_READ_LOG_DATA2 on progress ...");
    } else if(strcmp(g_in_cmd,"log1") == 0) {
        g_op_completed=0;
        buildcmd_logging(&g_fim5360,1);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_SET_LOG_OPTION on progress ...");
    } else if(strcmp(g_in_cmd,"log0") == 0) {
        g_op_completed=0;
        buildcmd_logging(&g_fim5360,0);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_SET_LOG_OPTION on progress ...");
    } else if(strcmp(g_in_cmd,"getlogopt") == 0) {
        g_op_completed=0;
        buildcmd_getlogopt(&g_fim5360);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_GET_LOG_OPTION on progress ...");
    } else if(strcmp(g_in_cmd,"getsysi") == 0) {
        g_op_completed=0;
//        buildcmd_getsysi(&g_fim5360,0x02);
        buildcmd_getsysi(&g_fim5360,0xF0);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_GET_SYS_INFO on progress ...");
    } else if(strcmp(g_in_cmd,"autoidon") == 0) {
        g_op_completed=0;
        buildcmd_autoid(&g_fim5360,1);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        g_autoidon=1;
        Serial.println("Serial command CMD_AUTO_IDENTIFY on progress ...");
    } else if(strcmp(g_in_cmd,"autoidoff") == 0) {
        g_op_completed=0;
        buildcmd_autoid(&g_fim5360,0);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        g_autoidon=0;
        Serial.println("Serial command CMD_AUTO_IDENTIFY on progress ...");
    } else if(strcmp(g_in_cmd,"deletefp") == 0) {
        ptr=(const unsigned char*)"0000\0\0\0\0\0\0\0\0\0\0";
//        ptr=(const unsigned char*)"0000\0\0\0\0\0\0\0\0\0\0";
        g_op_completed=0;
        buildcmd_deletefp(&g_fim5360,10);
        fim5360_send(&g_fim5360,ptr);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_DELETE_FP on progress ...");
    } else if(strcmp(g_in_cmd,"deleteallfp") == 0) {
        g_op_completed=0;
//0x00-all user, 0x01-all user except master, 0x02-delete all master, 0x03-format fp area
        buildcmd_deleteallfp(&g_fim5360,0x0);
        fim5360_send(&g_fim5360,ptr);
        g_do_time=g_millis;
        g_sop_time=10000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_DELETE_ALL_FP on progress ...");
    } else if(strcmp(g_in_cmd,"register") == 0) {
        g_op_completed=0;
        buildcmd_register(&g_fim5360,0,0x10,0);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        g_registerph=1;
        Serial.println("Serial command CMD_REGISTER_FP ph1 on progress ...");
    } else if(strcmp(g_in_cmd,"getimgq") == 0) {
        g_op_completed=0;
        buildcmd_getimagequality(&g_fim5360);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        g_autoidon=0;
        Serial.println("Serial command CMD_AUTO_IDENTIFY on progress ...");
    } else if(strcmp(g_in_cmd,"wud") == 0) {
        ptr=(const unsigned char*)"data of mine";
        g_op_completed=0;
        buildcmd_writeuserdata(&g_fim5360,0x0,strlen((const char*)ptr)+1);
        fim5360_send(&g_fim5360,ptr);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_WRITE_USER_DATA on progress ...");
    } else if(strcmp(g_in_cmd,"rud") == 0) {
        g_outtype=1;
        g_op_completed=0;
//        buildcmd_readuserdata(&g_fim5360,0x0,sizeof(g_recvbuf_d));
        buildcmd_readuserdata(&g_fim5360,0x0,25);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_READ_USER_DATA on progress ...");
    } else if(strcmp(g_in_cmd,"delmapwd") == 0) {
        g_op_completed=0;
        buildcmd_delmapwd(&g_fim5360);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        g_autoidon=0;
        Serial.println("Serial command CMD_DELETE_MASTER_PASSWORD on progress ...");
    } else if(strcmp(g_in_cmd,"gettime") == 0) {
        g_outtype=3;
        g_op_completed=0;
        buildcmd_gettime(&g_fim5360);
        fim5360_send(&g_fim5360,NULL);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_GET_TIME on progress ...");
    } else if(strcmp(g_in_cmd,"settime") == 0) {
        g_op_completed=0;
        getbcdtime((char*)"20160121171500",tmbuf);
        buildcmd_settime(&g_fim5360);
        fim5360_send(&g_fim5360,tmbuf);
        g_do_time=g_millis;
        g_sop_time=8000;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
        Serial.println("Serial command CMD_SET_TIME on progress ...");
    } else if(g_in_cmdlen == 1) {
      switch(g_in_cmd[0]) {
        case 'r':
          g_op_completed=0;
          g_hold_time=50;
          digitalWrite(FIM5360_REG_PORT,LOW);
          g_do_time=g_millis;
          g_do_port=FIM5360_REG_PORT;
          Serial.println("Registration ...");
          break;
        case 'd':
          g_op_completed=0;
          g_hold_time=50;
          digitalWrite(FIM5360_DEL_PORT,LOW);
          g_do_time=g_millis;
          g_do_port=FIM5360_DEL_PORT;
          Serial.println("Deletion by finger ...");
          break;
        case 'D':
          g_op_completed=0;
          g_hold_time=5000;
          digitalWrite(FIM5360_DEL_PORT,LOW);
          g_do_time=g_millis;
          g_do_port=FIM5360_DEL_PORT;
          Serial.println("Delete all  ...");
          break;
        case 'i':
          g_op_completed=0;
          g_hold_time=50;
          digitalWrite(FIM5360_IDENT_PORT,LOW);
          g_do_time=g_millis;
          g_do_port=FIM5360_IDENT_PORT;
          Serial.println("Identification ...");
          break;
        default:
          Serial.println("ignored1");
          break;
      }
    } else {
      Serial.print(g_in_cmd);
      Serial.print(" ");
      Serial.println("ignored");
    }
    g_in_cmd[0]='\0';
    g_in_cmdlen=0;
  }
  if((g_fim5360.u.hcs.hdt.cmd != 0) || (g_autoidactive == 1)) {
    if((g_fim5360.u.hcs.hdt.cmd == 0) && (g_autoidactive == 1)) {
    }
//    if((ret=fim5360_read(&g_fim5360r,NULL,0)) == 1) {
    if((ret=fim5360_read(&g_fim5360r,g_recvbuf_d,sizeof(g_recvbuf_d))) == 1) {
      if(g_registerph == 2) {
        g_registerph=0;
      }
      swap(&g_fim5360r.u.hcs.hdt.cmd);
      Serial.print("Serial command 0x");
      Serial.print(g_fim5360r.u.hcs.hdt.cmd,HEX);
      Serial.println(" processed!");
      Serial.print("cmd: 0x");
      Serial.print(g_fim5360r.u.hcs.hdt.cmd,HEX);
      swap(&g_fim5360r.u.hcs.hdt.p1);
      res=g_fim5360r.u.hcs.hdt.p1;
      Serial.print(" p1: ");
      Serial.print(g_fim5360r.u.hcs.hdt.p1);
      swap(&g_fim5360r.u.hcs.hdt.p2);
      Serial.print(" p2: ");
      Serial.print(g_fim5360r.u.hcs.hdt.p2,HEX);
      Serial.print(" ds: ");
      Serial.print(g_fim5360r.u.hcs.hdt.ds);
      Serial.print(" ec: ");
      swap(&g_fim5360r.u.hcs.hdt.ec);
      ec=g_fim5360r.u.hcs.hdt.ec;
      Serial.println(g_fim5360r.u.hcs.hdt.ec);
      if(g_fim5360r.u.hcs.hdt.ds != 0) {
        Serial.print("Data available: ");
        Serial.print(g_fim5360r.u.hcs.hdt.ds);
        for(i=0;i < g_fim5360r.u.hcs.hdt.ds;i++) {
          Serial.print(" ");
          Serial.print(g_recvbuf_d[i]);
        }
        Serial.println("");
        if((g_recvbuf_d[0] == '0') &&
           (g_recvbuf_d[1] == '0') &&
           (g_recvbuf_d[2] == '0') &&
           (g_recvbuf_d[3] == '5')) {
           Serial.println("Kelen");
         } else {
           Serial.println("Papi");
         }
        if(g_outtype == 1) {
          Serial.print("[");
          for(i=0;i < g_fim5360r.u.hcs.hdt.ds;i++) {
            Serial.print((char)g_recvbuf_d[i]);
          }
          Serial.println("]");
        }
        if(g_outtype == 2) {
          for(i=0;i < g_fim5360r.u.hcs.hdt.ds;i++) {
            if(g_recvbuf_d[i] < 10) {
              Serial.print("0");
            }
            Serial.print(g_recvbuf_d[i],HEX);
          }
          Serial.println("");
        }
        if(g_outtype == 3) {
          for(i=0;i < g_fim5360r.u.hcs.hdt.ds;i++) {
            if(g_recvbuf_d[i] < 10) {
              Serial.print("0");
            }
            Serial.print(g_recvbuf_d[i],HEX);
            if(i == 1) Serial.print(".");
            else if(i == 2) Serial.print(".");
            else if(i == 3) Serial.print(" ");
            else if(i == 4) Serial.print(":");
            else if(i == 5) Serial.print(":");
            else if(i == 6) Serial.print("/");
          }
          Serial.println("");
        }
      }
      if(g_fim5360.u.hcs.hdt.cmd == CMD_AUTO_IDENTIFY) {
        if(g_autoidon == 0) g_autoidactive=0;
        else if(g_autoidon == 1) g_autoidactive=1;
      }
      g_fim5360.u.hcs.hdt.cmd=0;
      g_do_time=0;
      g_op_completed=1;
      g_fim5360r.sdr=0;
      g_fim5360r.lead=0;
      if(g_registerph == 1) {
        if((ec == ERR_NONE) && (res == RESULT_SUCCEEDED)) {
          g_registerph=2;
          g_op_completed=0;
          buildcmd_register(&g_fim5360,0,0x01,0);
          fim5360_send(&g_fim5360,NULL);
          g_do_time=g_millis;
          g_sop_time=8000;
          g_fim5360r.sdr=0;
          g_fim5360r.lead=0;
          Serial.println("Serial command CMD_REGISTER_FP ph2 on progress ...");
        } else {
          g_registerph=0;
        }
      }
    } else {
      if(ret != 0) {
        if(ret == -4) {
        }
        g_registerph=0;
        Serial.print("Serial command ");
        Serial.print(g_fim5360.u.hcs.hdt.cmd,HEX);
        Serial.print(" failed: ");
        Serial.print(ret);
        Serial.println("!");
        g_fim5360.u.hcs.hdt.cmd=0;
        g_do_time=0;
        g_op_completed=1;
        g_fim5360r.sdr=0;
        g_fim5360r.lead=0;
      }
    }
    if(g_do_time > 0) {
      if(g_do_time+g_sop_time <= g_millis) {
        Serial.print("Serial command ");
        Serial.print(g_fim5360.u.hcs.hdt.cmd,HEX);
        Serial.println(" timed out!");
        g_fim5360.u.hcs.hdt.cmd=0;
        g_do_time=0;
        g_op_completed=1;
      }
    }
  }
  if(g_do_port != 0) {
    if(g_do_time+g_hold_time <= g_millis) {
      digitalWrite(g_do_port,HIGH);
    Serial.println("go HIGH");
      g_do_port=0;
      g_do_time=0;
    }
  }
  g_op_succ=digitalRead(FIM5360_SUCC_PORT);
  g_op_fail=digitalRead(FIM5360_FAIL_PORT);
  if(g_op_succ == HIGH) {
    if(g_op_time == 0) {
//      Serial.print(g_autoidactive);
      Serial.println("SUCCESS");
      g_op_time=g_millis;
      g_op_completed=1;
    } else {
      if(g_op_time+1100 < g_millis) {
        g_op_time=0;
      }
    }
  }
  if(g_op_fail == HIGH) {
    if(g_op_time == 0) {
//      Serial.print(g_autoidactive);
      Serial.println("FAIL");
      g_op_time=g_millis;
      g_op_completed=1;
    } else {
      if(g_op_time+1100 < g_millis) {
        g_op_time=0;
      }
    }
  }
}

