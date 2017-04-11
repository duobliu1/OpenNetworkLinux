/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
 *           Copyright 2016 Accton Technology Corporation.
 *           Copyright 2017 Agema System, Inc
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <onlplib/file.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>

#include "x86_64_agema_ag7648_int.h"
#include "x86_64_agema_ag7648_log.h"

#include "platform_lib.h"
#include "x86_64_agema_i2c.h"
platform_id_t platform_id = PLATFORM_ID_UNKNOWN;

#define ONIE_PLATFORM_NAME "x86-64-agema-ag7648-r0"
const char*
onlp_sysi_platform_get(void)
{ 
	enum ag7648_product_id pid = get_product_id();
	
	if (pid == PID_AG7648)
		return "x86-64-agema-ag7648";
	else 
		return "unknow";
}

int
onlp_sysi_platform_set(const char* platform)
{ 
    if(strstr(platform,"x86-64-agema-ag7648-r0")) {
        platform_id = PLATFORM_ID_AGEMA_AG7648_R0;
        return ONLP_STATUS_OK;
    }
	AIM_LOG_ERROR("No support for platform '%s'", platform);
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{ 
    int   v;
    
	v = i2c_devname_read_byte("SYSCPLD", 0X0);
	
    pi->cpld_versions = aim_fstrdup("%d", v & 0xf);

    return 0;
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    uint8_t* rdata = aim_zmalloc(256);

#if 0    
    if (i2c_devname_read_block("ID_EEPROM", 0, (char*)rdata, 256) < 0)
    {
        aim_free(rdata);
        DEBUG_PRINT("size %d", *size);
        return ONLP_STATUS_E_INTERNAL;
    }
	int fd,rc_size;
	char  fullpath[20] = {0};
	
	sprintf(fullpath, "/dev/mtd7");
	
	fd=open(fullpath,O_RDWR);
	
	if(fd<0){
		aim_free(rdata);
		return ONLP_STATUS_E_INTERNAL ;
	}
		
	rc_size=read(fd,rdata,256);
	
	if(rc_size<0||rc_size!=256){
		aim_free(rdata);
		return ONLP_STATUS_E_INTERNAL ;
	}
#endif			
    *data = rdata;
    return ONLP_STATUS_OK;

	
}

void
onlp_sysi_onie_data_free(uint8_t* data)
{ 
    aim_free(data);
}



int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{ 
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    /* 1 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* LEDs on the chassis */
    for (i = 1; i <= chassis_led_count(); i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 1 Fans on the chassis */
    for (i = 1; i <= chassis_fan_count(); i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    return 0;
}

int
onlp_sysi_platform_manage_fans(void)
{ 

    int rc;
    onlp_thermal_info_t ti2, ti3, ti4;
    int mtemp=0;
    int new_rpm=0;

    if (chassis_fan_count() == 0) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }
  
    /* Get temperature */
    /*rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(1), &ti1);
	
    if (rc != ONLP_STATUS_OK) {
        return rc;
    }*/
        
    rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(2), &ti2);
	
    if (rc != ONLP_STATUS_OK) {
        return rc;
    }
    rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(3), &ti3);
	
    if (rc != ONLP_STATUS_OK) {
        return rc;
    }
        
    rc = onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(4), &ti4);
	
    if (rc != ONLP_STATUS_OK) {
        return rc;
    }
             
    mtemp=(ti2.mcelsius+ti3.mcelsius + ti4.mcelsius) / 3;

    DEBUG_PRINT("mtemp %d\n", mtemp);

    /* Bring fan speed according the temp
     */
    if(mtemp<25000)
        new_rpm=FAN_IDLE_RPM;
    else if((mtemp>=30000)&&(mtemp<40000))
        new_rpm=FAN_LEVEL1_RPM;
    else if((mtemp>=45000)&&(mtemp<55000))
        new_rpm=FAN_LEVEL2_RPM;
    else if((mtemp>=60000)&&(mtemp<75000))
        new_rpm=FAN_LEVEL3_RPM;
    else if(mtemp>=80000)
        new_rpm=FAN_LEVEL4_RPM;
    else{
        return ONLP_STATUS_OK;
   }
       
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(1),new_rpm);
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(2),new_rpm); 
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(3),new_rpm); 
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(4),new_rpm); 
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(5),new_rpm); 
    onlp_fani_rpm_set(ONLP_FAN_ID_CREATE(6),new_rpm); 
     
    return ONLP_STATUS_OK;
}


int
onlp_sysi_platform_manage_leds(void)
{ 
		int rc,rc1;
		
		onlp_fan_info_t info1,info2;
		onlp_led_mode_t fan_new_mode;
		onlp_psu_info_t psu1;
		onlp_led_mode_t psu1_new_mode;
		onlp_psu_info_t psu2;
		onlp_led_mode_t psu2_new_mode;
		onlp_led_mode_t sys_new_mode;
		/*fan led */
		rc=onlp_fani_info_get(ONLP_FAN_ID_CREATE(1), &info1);
        
        rc1=onlp_fani_info_get(ONLP_FAN_ID_CREATE(2), &info2);
        
		if ((rc != ONLP_STATUS_OK)||(rc1 != ONLP_STATUS_OK)){
			fan_new_mode=ONLP_LED_MODE_RED;
			goto psu1_led;
		}
		if(((info1.status&0x3)==1)&&((info2.status&0x3)==1))
			fan_new_mode=ONLP_LED_MODE_GREEN;
		else 
			fan_new_mode=ONLP_LED_MODE_RED;
       
psu1_led:		
		onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN),fan_new_mode);
		/*psu1 and psu2 led */
		rc=onlp_psui_info_get(ONLP_PSU_ID_CREATE(1),&psu1);
		
		if (rc != ONLP_STATUS_OK) {
			psu1_new_mode=ONLP_LED_MODE_OFF;
			goto psu2_led;
		}
		
		if((psu1.status&0x1)&&!(psu1.status&0x2))
			psu1_new_mode=ONLP_LED_MODE_GREEN;
		else
			psu1_new_mode=ONLP_LED_MODE_OFF;
psu2_led:		
		onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_PSU1),psu1_new_mode);
		//psu2 led 	----------------
			rc=onlp_psui_info_get(ONLP_PSU_ID_CREATE(2),&psu2);
			
		 if (rc != ONLP_STATUS_OK) {
			psu2_new_mode=ONLP_LED_MODE_OFF;
			goto sys_led;
		}
		
		if((psu2.status&0x1)&&!(psu2.status&0x2))
			psu2_new_mode=ONLP_LED_MODE_GREEN;
		else
			psu2_new_mode=ONLP_LED_MODE_OFF;
sys_led	:		
		onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_PSU2),psu2_new_mode);
		//sys led 	----------------

		if((fan_new_mode!=ONLP_LED_MODE_GREEN)||(psu2_new_mode!=ONLP_LED_MODE_GREEN)|| \
			(psu1_new_mode!=ONLP_LED_MODE_GREEN))
			sys_new_mode=ONLP_LED_MODE_RED_BLINKING;
		else
			sys_new_mode=ONLP_LED_MODE_GREEN;
		
		onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_SYS),sys_new_mode);

		return ONLP_STATUS_OK;
}

int
onlp_sysi_onie_info_get(onlp_onie_info_t* onie)
{
	
    if(onie->platform_name) {
           aim_free(onie->platform_name);
    }
    onie->platform_name = aim_strdup(ONIE_PLATFORM_NAME);
    return ONLP_STATUS_OK;
}
