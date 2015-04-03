
#ifdef __BCM_SUPPORT__


#ifdef __QUECTEL_BCM_AT_CMD__
#include <stdio.h>
#include <string.h>
#include "stack_config.h"
#include "kal_trace.h"
#include "kal_general_types.h"
#include "kal_public_api.h"
//#include "defs.h"
#ifndef FALSE
#define FALSE          0
#define TRUE           1
#endif

/*
 * Definitions for NULL
 */
#ifndef NULL
#define NULL           0
#endif


/***************************************************************************** 
* Included files
*****************************************************************************/
#include "Bcm_btcmSrvProt.h"
#include "Bcm_btcmSrvGprot.h"
#include "bcm_trc.h"
#include "bluetooth_bm_struct.h"



#include "l4c_common_enum.h"
#include "rmmi_common_enum.h"
#include "rmmi_context.h"
#include "l4c_context.h"
#include "layer4_context.h"

#include "l4_trc.h"
#include "atci_trc.h"
#include "l4c_utility.h"


#include "rmmi_rspfmttr.h"
#include "rmmi_utility.h"
#include "rmmi_sio.h"


#include"dlist.h"
#include"rmmi_ql_bt_at_utils.h"
#include"bcmadp.h"

LIST_HEAD(g_ql_bt_dev_list);
LIST_HEAD(g_ql_bt_paired_dev_list);
LIST_HEAD(g_ql_bt_connected_dev_list);

ql_bt_dev_info_struct g_bt_dev_temp = {0};


const ql_bt_profile_struct g_ql_bt_profile_name_map[6] = 
{
    {"SPP\0",0x1101,},
	{"OBEX_PBA_PROFILE_CLIENT\0",0x112E,},
	{"OBEX_PBA_PROFILE",0x112F,},
	{"OBEX_OBJECT_PUSH_SERVICE",0x1105,},
	{"OBEX_OBJECT_PUSH_CLIENT",0xfffd,},
	{"HF_PROFILE",0x111E,}

};

void bt_utils_set_dev_element(kal_uint8 device_id,kal_uint8 * addr,kal_uint8* name_string,kal_uint8 name_len,kal_uint32 profile_uuid,kal_uint8 role,UART_PORT port)
{
	if(255 != device_id)
	{
		g_bt_dev_temp.device_id = device_id;		
	}
	kal_prompt_trace(MOD_ATCI, "victor set g_bt_dev_temp.device_id:%d",device_id);
	
	g_bt_dev_temp.profile_uuid = profile_uuid;
	if(255 != role)
	g_bt_dev_temp.role = role;

	if(255 != port)
	g_bt_dev_temp.port = port;
	
	kal_prompt_trace(MOD_ATCI, "victor set device_id:%d,port:%d",g_bt_dev_temp.device_id,g_bt_dev_temp.port);
	
	if(addr != NULL )
	{
		kal_mem_cpy((kal_uint8*) g_bt_dev_temp.addr, (kal_uint8*) addr, 6);
	}

	if(name_string != NULL)
	{
		//name	 
		kal_mem_set(g_bt_dev_temp.name_string, 0, BTBM_ADP_MAX_NAME_LEN);
		kal_mem_cpy(g_bt_dev_temp.name_string, name_string, (name_len<BTBM_ADP_MAX_NAME_LEN)?name_len:BTBM_ADP_MAX_NAME_LEN);
	}
	return;

}

void bt_utils_get_dev_element(ql_bt_dev_info_struct *p_dev)
{
	if(p_dev != NULL)
	{
		kal_mem_cpy(p_dev,&g_bt_dev_temp,sizeof(ql_bt_dev_info_struct));	
	}

}

void bt_utils_set_dev_element_port(UART_PORT port)
{
	g_bt_dev_temp.port = port;
	kal_prompt_trace(MOD_ATCI, "victor set port:%d",g_bt_dev_temp.port);
}
UART_PORT bt_utils_get_dev_element_port(void)
{
	return g_bt_dev_temp.port;
}

void bt_utils_set_dev_element_id(kal_uint8 id)
{
	g_bt_dev_temp.device_id = id;
	kal_prompt_trace(MOD_ATCI, "victor set g_bt_dev_temp.device_id:%d",g_bt_dev_temp.device_id);
	//kal_prompt_trace(MOD_ATCI, "victor set port:%d",g_bt_dev_temp.port);
}
kal_uint8 bt_utils_get_dev_element_id(void)
{
	kal_prompt_trace(MOD_ATCI, "victor get g_bt_dev_temp.device_id:%d",g_bt_dev_temp.device_id);
	return g_bt_dev_temp.device_id;
}
void bt_utils_set_dev_element_profile_uuid(kal_uint32 uuid)
{
	g_bt_dev_temp.profile_uuid = uuid;
}
kal_uint32 bt_utils_get_dev_element_profile_uuid(void)
{
	return g_bt_dev_temp.profile_uuid;
}

void bt_utils_set_dev_element_invalid(void)
{
	g_bt_dev_temp.device_id = 0;
	kal_prompt_trace(MOD_ATCI, "victor reset g_bt_dev_temp.device_id:%d",g_bt_dev_temp.device_id);
}

void bt_utils_get_dev_element_name(kal_uint8 *p_name)
{
	kal_uint32 name_len = strlen(g_bt_dev_temp.name_string);
	if(p_name != NULL)
	{
		//name	 
		kal_mem_set(p_name, 0, BTBM_ADP_MAX_NAME_LEN);
		kal_mem_cpy(p_name, g_bt_dev_temp.name_string, (name_len<BTBM_ADP_MAX_NAME_LEN)?name_len:BTBM_ADP_MAX_NAME_LEN);
	}
}
void bt_utils_get_dev_element_addr(kal_uint8 *p_devout)
{
	if(p_devout != NULL)
	kal_mem_cpy(p_devout,g_bt_dev_temp.addr,6);
}
void bt_utils_print_list(struct list_head *head,kal_uint8 type)
{
    kal_uint8 buffer[MAX_UART_LENGTH];
	kal_uint8 outaddr[13];
	kal_uint8 string_length;
	struct list_head * pos;
	ql_bt_dev_info_struct *p_paired_dev = NULL;
	ql_bt_dev_info_struct *p_connected_dev = NULL;
	if(list_empty(head))
	{
		//print
		/*
		kal_sprintf((kal_char*) buffer, "list empty!");
		string_length = strlen((char*)buffer);
		rmmi_write_to_uart(buffer, string_length, KAL_TRUE);
		*/
		return;
	}
	list_for_each(pos, head)
	{
		if(type == 1)
		{
			p_paired_dev = list_entry(pos, ql_bt_dev_info_struct, list);	
			//convert_btaddr_to_addrArrary(p_tmp->addr,&param_ptr->bd_addr);
	    	print_hex_value_ext(p_paired_dev->addr,outaddr,6);
			//kal_sprintf((kal_char*) buffer, "P:%d,%s,%s",p_paired_dev->device_id,p_paired_dev->name_string,outaddr);
			kal_sprintf((kal_char*) buffer, "+QBTSTATE:0,%d,\"%s\",%s",p_paired_dev->device_id,p_paired_dev->name_string,outaddr);
		}
		else
		{
			p_connected_dev = list_entry(pos, ql_bt_dev_info_struct, list);
			print_hex_value_ext(p_connected_dev->addr,outaddr,6);
			//kal_sprintf((kal_char*) buffer, "C:%d,%s,%s,%s,port:%d,role:%d",p_connected_dev->device_id,p_connected_dev->name_string,outaddr,g_ql_bt_profile_name_map[p_connected_dev->profile_index].name,p_connected_dev->port,p_connected_dev->role);
			//kal_sprintf((kal_char*) buffer, "1:%d,%s,%s,%s,port:%d,role:%d",p_connected_dev->device_id,p_connected_dev->name_string,outaddr,g_ql_bt_profile_name_map[p_connected_dev->profile_index].name,p_connected_dev->port,p_connected_dev->role);
			kal_sprintf((kal_char*) buffer, "+QBTSTATE:1,%d,\"%s\",%s,%s",p_connected_dev->device_id,p_connected_dev->name_string,outaddr,g_ql_bt_profile_name_map[p_connected_dev->profile_index].name);
		}

		string_length = strlen((char*)buffer);
		rmmi_write_to_uart(buffer, string_length, KAL_TRUE);
	}
	
	//kal_prompt_trace(MOD_ATCI, "victor list_for_each search over");

	return ;
}



kal_uint8* bt_utils_find_name_from_addr(struct list_head *head, kal_uint8 * p_addr)
{
	kal_uint8 * name_ret=NULL;
	struct list_head * pos;
	ql_bt_dev_info_struct *p_tmp;
	//p_addr = addr;
	list_for_each(pos, head)
	{
		p_tmp = list_entry(pos, ql_bt_dev_info_struct, list);
		if((*p_addr) == p_tmp->addr[0] && (*(p_addr+1)) == p_tmp->addr[1] && (*(p_addr+2)) == p_tmp->addr[2]\
			&&(*(p_addr+3)) == p_tmp->addr[3] && (*(p_addr+4)) == p_tmp->addr[4] && (*(p_addr+5)) == p_tmp->addr[5])
		{
			name_ret = p_tmp->name_string;
			kal_prompt_trace(MOD_ATCI, "victor find name:%s",name_ret);
		}
		kal_prompt_trace(MOD_ATCI, "victor searched name:%s",p_tmp->name_string);
	}
	
	kal_prompt_trace(MOD_ATCI, "victor list_for_each search over");

	return name_ret;


}
kal_uint8 bt_utils_find_id_from_addr(struct list_head *head, kal_uint8 * p_addr)
{
	struct list_head * pos;
	ql_bt_dev_info_struct *p_tmp;
	//p_addr = addr;
	list_for_each(pos, head)
	{
		p_tmp = list_entry(pos, ql_bt_dev_info_struct, list);
		if((*p_addr) == p_tmp->addr[0] && (*(p_addr+1)) == p_tmp->addr[1] && (*(p_addr+2)) == p_tmp->addr[2]\
			&&(*(p_addr+3)) == p_tmp->addr[3] && (*(p_addr+4)) == p_tmp->addr[4] && (*(p_addr+5)) == p_tmp->addr[5])
		{
			return p_tmp->device_id;
		}
		
	}
	
	kal_prompt_trace(MOD_ATCI, "victor list_for_each search over");

	return 255;


}

ql_bt_dev_info_struct * bt_utils_find_dev_node_from_addr(struct list_head *head, kal_uint8 * p_addr)
{
	struct list_head * pos;
	ql_bt_dev_info_struct *p_tmp = NULL;
	//p_addr = addr;
	list_for_each(pos, head)
	{
		p_tmp = list_entry(pos, ql_bt_dev_info_struct, list);
		if((*p_addr) == p_tmp->addr[0] && (*(p_addr+1)) == p_tmp->addr[1] && (*(p_addr+2)) == p_tmp->addr[2]\
			&&(*(p_addr+3)) == p_tmp->addr[3] && (*(p_addr+4)) == p_tmp->addr[4] && (*(p_addr+5)) == p_tmp->addr[5])
		{
			  return p_tmp;
		}
	}
	
	return NULL;

}
ql_bt_dev_info_struct * bt_utils_find_dev_node_from_id(struct list_head *head, kal_uint8 id)
{
	struct list_head * pos;
	ql_bt_dev_info_struct *p_tmp = NULL;
	
	list_for_each(pos, head)
	{
		p_tmp = list_entry(pos, ql_bt_dev_info_struct, list);
		if(id == p_tmp->device_id)
		{
			  return p_tmp;
		}
	}
	
	return NULL;

}
ql_bt_dev_info_struct * bt_utils_find_dev_node_from_port(struct list_head *head, UART_PORT port)
{
	struct list_head * pos;
	ql_bt_dev_info_struct *p_tmp = NULL;
	
	list_for_each(pos, head)
	{
		p_tmp = list_entry(pos, ql_bt_dev_info_struct, list);
		if(port == p_tmp->port)
		{
			  return p_tmp;
		}
	}
	
	return NULL;

}
kal_uint8 bt_utils_profile_find_index_from_uuid(kal_uint32 uuid)
{
	kal_uint8 i=0;
	for(i=0;i<6;i++)
	{
		if(g_ql_bt_profile_name_map[i].profile_uuid == uuid)
		{
			return i;
		}
	}
	return 255;
}


/*****************************************************************************
 * FUNCTION
 *  
 * DESCRIPTION
 *  add ql_bt_dev_info_struct type node.
 * PARAMETERS
    
 * RETURNS
 *  the pointer of the add node 
 *****************************************************************************/
ql_bt_dev_info_struct * bt_utils_add_dev_node(struct list_head *head, kal_uint8 * p_addr,kal_uint8 *p_name,kal_uint32 profile_uuid,UART_PORT port,kal_uint8 role)
{
	struct list_head * pos=NULL;
	kal_uint8 name_len = 0;
	ql_bt_dev_info_struct *p_dev_add,*p_dev_pre;
	if(head == NULL || p_addr == NULL || p_name == NULL)
	{
		kal_prompt_trace(MOD_ATCI, "victor input param error");
		return NULL;
	}
	//check if already existed
	if(head == &g_ql_bt_connected_dev_list)//spp connected can have two channel in one device 
	{

	}
	else if(NULL != bt_utils_find_name_from_addr( head, p_addr))
	{
		kal_prompt_trace(MOD_ATCI, "victor already existed");
		return NULL;
	}
	
	//generate
	p_dev_add= ( ql_bt_dev_info_struct *)OslMalloc(sizeof(ql_bt_dev_info_struct));
	if(p_dev_add == NULL)
	{
		kal_prompt_trace(MOD_ATCI, "victor add OslMalloc failed");
		return NULL;
	}
	
	//set node param
	if(list_empty(head))//firt pair node set id =1
	{
		p_dev_add->device_id = 1;
	}
	else
	{
		pos = head->prev;
		p_dev_pre= list_entry(pos, ql_bt_dev_info_struct, list);//get prev node
		p_dev_add->device_id= p_dev_pre->device_id+1;//id+
	}
	name_len = strlen(p_name);
	kal_mem_set(p_dev_add->name_string, 0, BTBM_ADP_MAX_NAME_LEN);	
	kal_mem_cpy(p_dev_add->name_string, p_name, (name_len<BTBM_ADP_MAX_NAME_LEN)?name_len:BTBM_ADP_MAX_NAME_LEN);
	kal_mem_cpy(p_dev_add->addr,p_addr,6);
	p_dev_add->profile_index = bt_utils_profile_find_index_from_uuid(profile_uuid);
	p_dev_add->profile_uuid = profile_uuid;
	p_dev_add->role = role;
	p_dev_add->port = port;
	
 	//error free node
	if((p_dev_add->profile_index == 255) || (p_dev_add->name_string == NULL))
	{
		OslMfree(p_dev_add);
		kal_prompt_trace(MOD_ATCI, "victor add error param %d %s",p_dev_add->profile_index,p_dev_add->name_string);
		return NULL;
	}
	
	//add node
	list_add_tail(&(p_dev_add->list), head);
	kal_prompt_trace(MOD_ATCI, "victor add success:%s",p_dev_add->name_string);

	return p_dev_add;
	
}

ql_bt_dev_info_struct * bt_utils_add_head_dev_node(struct list_head *head, kal_uint8 * p_addr,kal_uint8 *p_name,kal_uint32 profile_uuid,UART_PORT port,kal_uint8 role)
{
	struct list_head * pos=NULL;
	kal_uint8 name_len = 0;
	ql_bt_dev_info_struct *p_dev_add,*p_dev_next;
	if(head == NULL || p_addr == NULL || p_name == NULL)
	{
		kal_prompt_trace(MOD_ATCI, "victor input param error");
		return NULL;
	}
	//check if already existed
	if(head == &g_ql_bt_connected_dev_list)//spp connected can have two channel in one device 
	{

	}
	else if(NULL != bt_utils_find_name_from_addr( head, p_addr))
	{
		kal_prompt_trace(MOD_ATCI, "victor already existed");
		return NULL;
	}
	
	//generate
	p_dev_add= ( ql_bt_dev_info_struct *)OslMalloc(sizeof(ql_bt_dev_info_struct));
	if(p_dev_add == NULL)
	{
		kal_prompt_trace(MOD_ATCI, "victor add OslMalloc failed");
		return NULL;
	}
	
	//set node param
	if(list_empty(head))//firt pair node set id =1
	{
		p_dev_add->device_id = 1;
	}
	else
	{
		pos = head->next;
		p_dev_next= list_entry(pos, ql_bt_dev_info_struct, list);//get prev node
		p_dev_add->device_id= p_dev_next->device_id-1;//id
	}
	name_len = strlen(p_name);
	kal_mem_set(p_dev_add->name_string, 0, BTBM_ADP_MAX_NAME_LEN);	
	kal_mem_cpy(p_dev_add->name_string, p_name, (name_len<BTBM_ADP_MAX_NAME_LEN)?name_len:BTBM_ADP_MAX_NAME_LEN);
	kal_mem_cpy(p_dev_add->addr,p_addr,6);
	p_dev_add->profile_index = bt_utils_profile_find_index_from_uuid(profile_uuid);
	p_dev_add->profile_uuid = profile_uuid;
	p_dev_add->role = role;
	p_dev_add->port = port;
	
 	//error free node
	if((p_dev_add->profile_index == 255) || (p_dev_add->name_string == NULL))
	{
		OslMfree(p_dev_add);
		kal_prompt_trace(MOD_ATCI, "victor add error param %d %s",p_dev_add->profile_index,p_dev_add->name_string);
		return NULL;
	}
	
	//add node
	list_add(&(p_dev_add->list), head);
	kal_prompt_trace(MOD_ATCI, "victor add success:%s",p_dev_add->name_string);

	return p_dev_add;
	
}

/*****************************************************************************
 * FUNCTION
 *  
 * DESCRIPTION
 *  insert ql_bt_dev_info_struct type node.
 * PARAMETERS
    
 * RETURNS
 *  the pointer of the add node 
 *****************************************************************************/
ql_bt_dev_info_struct * bt_utils_insert_dev_node(struct list_head *prev,struct list_head *next, kal_uint8 * p_addr,kal_uint8 *p_name,kal_uint32 profile_uuid,UART_PORT port,kal_uint8 role)
{
	//struct list_head * pos=NULL;
	kal_uint8 name_len = 0;
	ql_bt_dev_info_struct *p_dev_add,*p_dev_pre;
	if(p_addr == NULL || p_name == NULL)
	{
		kal_prompt_trace(MOD_ATCI, "victor input param error");
		return NULL;
	}
	//check if already existed
	

	
	//generate
	p_dev_add= ( ql_bt_dev_info_struct *)OslMalloc(sizeof(ql_bt_dev_info_struct));
	if(p_dev_add == NULL)
	{
		kal_prompt_trace(MOD_ATCI, "victor add OslMalloc failed");
		return NULL;
	}
	
	//set node param

	
	p_dev_pre= list_entry(prev, ql_bt_dev_info_struct, list);//get prev node
	p_dev_add->device_id= p_dev_pre->device_id+1;//id+

	name_len = strlen(p_name);
	kal_mem_set(p_dev_add->name_string, 0, BTBM_ADP_MAX_NAME_LEN);	
	kal_mem_cpy(p_dev_add->name_string, p_name, (name_len<BTBM_ADP_MAX_NAME_LEN)?name_len:BTBM_ADP_MAX_NAME_LEN);
	kal_mem_cpy(p_dev_add->addr,p_addr,6);
	p_dev_add->profile_index = bt_utils_profile_find_index_from_uuid(profile_uuid);
	p_dev_add->profile_uuid = profile_uuid;
	p_dev_add->role = role;
	p_dev_add->port = port;
	
 	//error free node
	if((p_dev_add->profile_index == 255) || (p_dev_add->name_string == NULL))
	{
		OslMfree(p_dev_add);
		kal_prompt_trace(MOD_ATCI, "victor add error param %d %s",p_dev_add->profile_index,p_dev_add->name_string);
		return NULL;
	}
	
	//add node
	//list_add_tail(&(p_dev_add->list), head);
	__list_add(&(p_dev_add->list), prev, next);
	kal_prompt_trace(MOD_ATCI, "victor add success:%s",p_dev_add->name_string);

	return p_dev_add;
	
}

/*****************************************************************************
 * FUNCTION
 *  
 * DESCRIPTION
 *  find address accord id
 * PARAMETERS
 *  list head       [IN]        
 *  device_id       [IN]        
 * RETURNS
 *  char* address
 *****************************************************************************/
kal_uint8* bt_utils_find_addr_from_id(struct list_head *head, kal_uint8 device_id)
{
	kal_uint8 * addr_ret = NULL;
	struct list_head * pos;
	ql_bt_dev_info_struct *p_tmp;
	
	if(list_empty(head))
	{
	    kal_prompt_trace(MOD_ATCI, "victor list_empty!");

		return NULL;
	}
	list_for_each(pos, head)
	{
		p_tmp = list_entry(pos, ql_bt_dev_info_struct, list);
		if(device_id == p_tmp->device_id)
		{
			addr_ret = p_tmp->addr;
			//kal_prompt_trace(MOD_ATCI, "victor find name:%s",name_ret);
		}
	}
	
	//kal_prompt_trace(MOD_ATCI, "victor list_for_each search over");

	return addr_ret;

}

kal_bool bt_utils_free_dev_list(struct list_head *head)
{	
	ql_bt_dev_info_struct * st;
	struct list_head * pos,*n;

	pos = head;
	list_for_each_safe(pos,n,head)	
	{ 		
		st=list_entry(pos, ql_bt_dev_info_struct,list); 		
		list_del(pos); //ɾ\B3\FD\BDڵ㣬ɾ\B3\FD\BDڵ\E3\B1\D8\D0\EB\D4\DAɾ\B3\FD\BDڵ\E3\C4ڴ\E6֮ǰ 		
		OslMfree(st);  //\CAͷŽڵ\E3\C4ڴ\E6 	
	}
	return TRUE;
}


void bt_utils_get_exist_dev(void)
{
	kal_uint8 addr[6]={0};
	
    //got the total list and copy the to bd_addr
    kal_uint32 i, idx, total_num, max_total_number = 0;
    kal_uint8* idx_list = NULL;               
    total_num = g_srv_bt_cm_cntx.paired_dev_num;
    idx_list = g_srv_bt_cm_cntx.paired_idx_list;

    max_total_number = total_num < SRV_BT_CM_MAX_PAIRED_DEVICE_LIST ? total_num : SRV_BT_CM_MAX_PAIRED_DEVICE_LIST;

    //kal_prompt_trace(MOD_BCM,"case 4 max total number is %d",max_total_number);
    if(max_total_number > 0)
    {
        for (i = 0; i < max_total_number; i++)
        {
            idx = idx_list ? idx_list[i] : i;
            
			//addresss
			if(g_srv_bt_cm_cntx.dev_list[idx].dev_info.bd_addr.lap != 0)
			{
				convert_btaddr_to_addrArrary(addr,&g_srv_bt_cm_cntx.dev_list[idx].dev_info.bd_addr);
				bt_utils_add_dev_node(&g_ql_bt_paired_dev_list, addr, g_srv_bt_cm_cntx.dev_list[idx].dev_info.name, 0x1101, 0, 0);//profile uuid must exist
				kal_prompt_trace(MOD_ATCI,"bt_utils_get_exist_dev: %s",g_srv_bt_cm_cntx.dev_list[idx].dev_info.name);
			}
		}
    }
   return;
}

void bt_utils_add_paired_dev(kal_uint8 * addr,kal_uint8 * dev_name)
{
	struct list_head * pos;
	ql_bt_dev_info_struct *p_tmp = NULL,*p_tmp_prev =NULL;
	kal_uint8 id_gap = 0;
	
	list_for_each(pos, &g_ql_bt_paired_dev_list)
	{
		p_tmp = list_entry(pos, ql_bt_dev_info_struct, list);
		//p_tmp_next = list_entry(pos->next, ql_bt_dev_info_struct, list); 
		if(p_tmp_prev != NULL)
		{
			id_gap = p_tmp->device_id- p_tmp_prev->device_id ;
			kal_prompt_trace(MOD_ATCI,"victor debug id_gap:%d prev id:%d next id: %d",id_gap,p_tmp_prev->device_id,p_tmp->device_id);
			if((id_gap >= 2)   )
				break;
			//else if((id_gap == 0)   )
				//break;
		}
		
		p_tmp_prev = p_tmp;

	}

	if((id_gap >= 2)   )
	{
		bt_utils_insert_dev_node(&(p_tmp_prev->list),&(p_tmp->list), addr, dev_name, 0x1101, 0, 0);//profile uuid must exist

	}
	else //if((id_gap == 0))
	{
		//pos = g_ql_bt_paired_dev_list
		p_tmp = list_entry((g_ql_bt_paired_dev_list.next), ql_bt_dev_info_struct, list);
		if( (p_tmp->device_id != 1))
		{
			bt_utils_add_head_dev_node(&g_ql_bt_paired_dev_list, addr, dev_name, 0x1101, 0, 0);//profile uuid must exist
		}
		else
		{
			bt_utils_add_dev_node(&g_ql_bt_paired_dev_list, addr, dev_name, 0x1101, 0, 0);//profile uuid must exist
		}
	}

}
void bt_utils_update_paired_dev(kal_bool op)
{
	kal_uint8 addr[6]={0};
	kal_uint8 outaddr[13]={0};
	
    //got the total list and copy the to bd_addr
    kal_uint32 i, idx, total_num, max_total_number = 0;
    kal_uint8* idx_list = NULL;               
    
	struct list_head * pos;
	ql_bt_dev_info_struct *p_tmp = NULL;
	ql_bt_dev_info_struct * p_tmp_next = NULL;
	ql_bt_dev_info_struct * p_tmp_prev = NULL;
	struct list_head paired_dev_list_temp;
	kal_uint8 * p_addr = NULL;
	kal_uint8 id_gap = 0;
	kal_bool flag = TRUE;
	kal_prompt_trace(MOD_ATCI,"[victor debug] enter bt_utils_update_paired_dev op:%d",op);
	
	p_addr = addr;

	total_num = g_srv_bt_cm_cntx.paired_dev_num;
    idx_list = g_srv_bt_cm_cntx.paired_idx_list;

	
    max_total_number = total_num < SRV_BT_CM_MAX_PAIRED_DEVICE_LIST ? total_num : SRV_BT_CM_MAX_PAIRED_DEVICE_LIST;

	//bt_utils_free_dev_list(&g_ql_bt_paired_dev_list);

    if(op == FALSE)
	{
		list_for_each(pos, &g_ql_bt_paired_dev_list)
		{
			p_tmp = list_entry(pos, ql_bt_dev_info_struct, list);

			if(max_total_number >= 0)//0 for last one
			{
				for (i = 0; i < max_total_number; i++)
				{
				    idx = idx_list ? idx_list[i] : i;

					//addresss
					if(g_srv_bt_cm_cntx.dev_list[idx].dev_info.bd_addr.lap != 0)
					{
						convert_btaddr_to_addrArrary(addr,&g_srv_bt_cm_cntx.dev_list[idx].dev_info.bd_addr);
						print_hex_value_ext(addr,outaddr,6);
					    kal_prompt_trace(MOD_ATCI,"victor debug dev_list addr:%s",outaddr);
						if((*p_addr) == p_tmp->addr[0] && (*(p_addr+1)) == p_tmp->addr[1] && (*(p_addr+2)) == p_tmp->addr[2]\
				&&(*(p_addr+3)) == p_tmp->addr[3] && (*(p_addr+4)) == p_tmp->addr[4] && (*(p_addr+5)) == p_tmp->addr[5])
					{
					//\D6ظ\B4\B5\C4
					//return p_tmp->device_id;
						//break;
						goto label_del;
					}
					else
					{

					}
						//bt_utils_add_dev_node(&g_ql_bt_paired_dev_list, addr, g_srv_bt_cm_cntx.dev_list[idx].dev_info.name, 0x1101, 0, 0);//profile uuid must exist
						//kal_prompt_trace(MOD_ATCI,"bt_utils_get_exist_dev: %s",g_srv_bt_cm_cntx.dev_list[idx].dev_info.name);
					}
				}
			label_del:
				kal_prompt_trace(MOD_ATCI,"victor debug del i:%d,max_total_number:%d",i,max_total_number);
				if(i == max_total_number)//\D7\EE\BA\F3\B6\BCû\D5ҵ\BD
				{

					list_del(pos); //	
					OslMfree(p_tmp);  //\CAͷŽڵ\E3\C4ڴ\E6 
				}
			}

		}
	}
	else
	{
		if(max_total_number > 0)
		{
			for (i = 0; i < max_total_number; i++)
			{
			    idx = idx_list ? idx_list[i] : i;

				//addresss
				if(g_srv_bt_cm_cntx.dev_list[idx].dev_info.bd_addr.lap != 0)
				{
					convert_btaddr_to_addrArrary(addr,&g_srv_bt_cm_cntx.dev_list[idx].dev_info.bd_addr);
					print_hex_value_ext(addr,outaddr,6);
					kal_prompt_trace(MOD_ATCI,"victor debug dev_list addr:%s",outaddr);
					//flag = FALSE;
					
					
					list_for_each(pos, &g_ql_bt_paired_dev_list)
					{
						p_tmp = list_entry(pos, ql_bt_dev_info_struct, list);

						if((*p_addr) == p_tmp->addr[0] && (*(p_addr+1)) == p_tmp->addr[1] && (*(p_addr+2)) == p_tmp->addr[2]\
						&&(*(p_addr+3)) == p_tmp->addr[3] && (*(p_addr+4)) == p_tmp->addr[4] && (*(p_addr+5)) == p_tmp->addr[5])
						{
							//EXIST
							//flag  = TRUE;
							//break;//list_for_each
							goto label_end;
							
						}
						else
						{
							//goto label_add;
						}
					}
							
					if(1)
					{
						//add

					
							list_for_each(pos, &g_ql_bt_paired_dev_list)
							{
								p_tmp = list_entry(pos, ql_bt_dev_info_struct, list);
								//p_tmp_next = list_entry(pos->next, ql_bt_dev_info_struct, list); 
								if(p_tmp_prev != NULL)
								{
									id_gap = p_tmp->device_id- p_tmp_prev->device_id ;
									kal_prompt_trace(MOD_ATCI,"victor debug id_gap:%d prev id:%d next id: %d",id_gap,p_tmp_prev->device_id,p_tmp->device_id);
									if((id_gap >= 2)   )
										break;
									//else if((id_gap == 0)   )
										//break;
								}
								
								p_tmp_prev = p_tmp;

							}

							if((id_gap >= 2)   )
							{
								bt_utils_insert_dev_node(&(p_tmp_prev->list),&(p_tmp->list), addr, g_srv_bt_cm_cntx.dev_list[idx].dev_info.name, 0x1101, 0, 0);//profile uuid must exist

							}
							else //if((id_gap == 0))
							{
								//pos = g_ql_bt_paired_dev_list
								p_tmp = list_entry((g_ql_bt_paired_dev_list.next), ql_bt_dev_info_struct, list);
								if( (p_tmp->device_id != 1))
								{
									bt_utils_add_head_dev_node(&g_ql_bt_paired_dev_list, addr, g_srv_bt_cm_cntx.dev_list[idx].dev_info.name, 0x1101, 0, 0);//profile uuid must exist
								}
								else
								{
									bt_utils_add_dev_node(&g_ql_bt_paired_dev_list, addr, g_srv_bt_cm_cntx.dev_list[idx].dev_info.name, 0x1101, 0, 0);//profile uuid must exist
								}
							}

					}
					label_end:
				}
				
			}


		}

	}
	 
	return;
}

void bt_utils_update_paired_dev_old(void)
{
	kal_uint8 addr[6]={0};
	kal_uint8 outaddr[13] = {0};
	
    //got the total list and copy the to bd_addr
    kal_uint32 i, idx, total_num, max_total_number = 0;
    kal_uint8* idx_list = NULL;  

	kal_prompt_trace(MOD_ATCI,"[victor debug] enter bt_utils_update_paired_dev_old");
		
    total_num = g_srv_bt_cm_cntx.paired_dev_num;
    idx_list = g_srv_bt_cm_cntx.paired_idx_list;

    max_total_number = total_num < SRV_BT_CM_MAX_PAIRED_DEVICE_LIST ? total_num : SRV_BT_CM_MAX_PAIRED_DEVICE_LIST;

	//bt_utils_free_dev_list(&g_ql_bt_paired_dev_list);
    //kal_prompt_trace(MOD_BCM,"case 4 max total number is %d",max_total_number);
    if(max_total_number > 0)
    {
        for (i = 0; i < max_total_number; i++)
        {
            idx = idx_list ? idx_list[i] : i;

			//addresss
			if(g_srv_bt_cm_cntx.dev_list[idx].dev_info.bd_addr.lap != 0)
			{
				convert_btaddr_to_addrArrary(addr,&g_srv_bt_cm_cntx.dev_list[idx].dev_info.bd_addr);
				print_hex_value_ext(addr,outaddr,6);
				kal_prompt_trace(MOD_ATCI,"victor debug dev_list addr:%s",outaddr);
					
				bt_utils_add_dev_node(&g_ql_bt_paired_dev_list, addr, g_srv_bt_cm_cntx.dev_list[idx].dev_info.name, 0x1101, 0, 0);//profile uuid must exist
				kal_prompt_trace(MOD_ATCI,"bt_utils_get_exist_dev: %s",g_srv_bt_cm_cntx.dev_list[idx].dev_info.name);
			}
		}
    }
	return;
}

void bt_utils_write_urc(kal_uint8 *buffer, kal_uint16 length, kal_bool stuff)
{
	//extern void rmmi_write_to_uart(kal_uint8 *buffer, kal_uint16 length, kal_bool stuff);
	rmmi_write_to_uart(buffer,  length,  stuff);
	return;
}


void bt_utils_updata_dev_name(struct list_head *head, kal_uint8 * p_addr,kal_uint8 *p_name)
{
	kal_uint8 * name_ret=NULL;
	struct list_head * pos;
	ql_bt_dev_info_struct *p_tmp;
	//p_addr = addr;
	            kal_prompt_trace(MOD_ATCI, "Ramos find bt_utils_updata_dev_name:%s",p_name);
	list_for_each(pos, head)
	{
		p_tmp = list_entry(pos, ql_bt_dev_info_struct, list);
		if((*p_addr) == p_tmp->addr[0] && (*(p_addr+1)) == p_tmp->addr[1] && (*(p_addr+2)) == p_tmp->addr[2]\
			&&(*(p_addr+3)) == p_tmp->addr[3] && (*(p_addr+4)) == p_tmp->addr[4] && (*(p_addr+5)) == p_tmp->addr[5])
		{
			kal_mem_set(p_tmp->name_string,0x00, BTBM_ADP_MAX_NAME_LEN);
                   kal_mem_cpy(p_tmp->name_string,p_name, strlen(p_name));
			kal_prompt_trace(MOD_ATCI, "Ramos updata find name:%s",p_name);
		}

	}
}
#endif

#endif
