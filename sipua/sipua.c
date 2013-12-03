#include <pjsua.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#define THIS_FILE "sip.c"

int bShutdown = 0;

/* define callbacks */
void on_reg_started(pjsua_acc_id acc_id, pj_bool_t renew)
{
	printf("on register started\n");
}
void on_reg_state(pjsua_acc_id acc_id)
{
	printf("on register status\n");
}
void on_reg_state2(pjsua_acc_id acc_id, pjsua_reg_info *info)
{
	printf("on register status 2\n");
}

void on_call_state(pjsua_call_id call_id, pjsip_event *e)
{
	pjsua_call_info call_info;
	printf("on_call_status\n");
	pjsua_call_get_info(call_id, &call_info);
	if (call_info.state == PJSIP_INV_STATE_DISCONNECTED)
	{
		printf("PJSIP_INV_STATE_DISCONNECTED\n");
	}
}

void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id,
		                             pjsip_rx_data *rdata)
{
	printf("on_incoming_call:incoming call to acc[%d]\n", acc_id);
}

void on_pager2(pjsua_call_id call_id, const pj_str_t *from,
		const pj_str_t *to, const pj_str_t *contact,
		const pj_str_t *mime_type, const pj_str_t *body,
		pjsip_rx_data *rdata, pjsua_acc_id acc_id)
{
	printf("message from %s\n", from->ptr);
}

static void
signalHandler(int signo)
{
	printf("system is going to shutdown!\n");
	bShutdown = 1;
}

pjsip_redirect_op on_call_redirected(pjsua_call_id call_id,
		const pjsip_uri *target,
		const pjsip_event *e)
{
	return PJSIP_REDIRECT_ACCEPT;
}

static pthread_cond_t _cond;
static pthread_mutex_t _mutex;

void *test_thread(void *param)
{
	int aid = *(int *)param;
	pj_thread_t *thread;
	pj_thread_desc desc;
	pj_status_t rc = pj_thread_register ( "test_thread",
			desc,
			&thread);

	pthread_mutex_lock(&_mutex);
	printf("*******************************\n");
	printf("test thread started\n");
	printf("*******************************\n");
	pthread_cond_wait(&_cond, &_mutex);
	pj_str_t to = pj_str("sip:user1@192.168.1.197");
	pj_str_t mime_type = pj_str("text/plain");
	pj_str_t content = pj_str("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<SIP_XML EventType=\"TimeSync\"/>"
			"<Item operation=\"query\" time=\"0-0-0 0:0:0\"/>"
			"</SIP_XML>");
	pjsua_im_send(aid, &to, &mime_type, &content, NULL, NULL);
	printf("*******************************\n");
	printf("pjsua_im_send ok @ test_thread\n");
	printf("*******************************\n");
	pthread_mutex_unlock(&_mutex);
	return NULL;
}

//void make_call()
//{
//	pj_status_t status;
//	pj_str_t dst_uri;
//	pjsua_msg_data msg_data;
//	pjsua_call_setting call_setting;
//
//	status = pjsua_verify_url(call_cfg->req_uri.ptr);
//	if (status != PJ_SUCCESS) 
//	{
//		PJ_LOG(1,(THIS_FILE, ">>> Error verify req_uri in xpjsip_call_make_call"));
//		return -1;
//	}
//	dst_uri = pj_str(call_cfg->req_uri.ptr);
//
//	pjsua_msg_data_init(&msg_data);
//	if (call_cfg->msg_body.mime.len && call_cfg->msg_body.body.len)
//	{
//		pj_cstr(&msg_data.content_type, call_cfg->msg_body.mime.ptr);
//		pj_cstr(&msg_data.msg_body, call_cfg->msg_body.body.ptr);
//	}
//	
//	pjsua_call_setting_default(&call_setting);
//	status = pjsua_call_make_call(acc_id, &dst_uri, &call_setting, NULL, &msg_data, call_id);
//	if (status != PJ_SUCCESS) 
//	{
//		PJ_LOG(1,(THIS_FILE, ">>> Error make call in xpjsip_call_make_call"));
//		return -1;
//	}
//
//	return 0;
//}

int main()
{
	/* system variables */
	pj_status_t status;
	pjsua_config			ua_cfg;
	pjsua_acc_config		acc_cfg;
	pjsua_logging_config	log_cfg;
	pjsua_media_config		media_cfg;
	pjsua_transport_config	udp_cfg;
	pjsua_transport_id		transport_id;
	pjsua_acc_id			local_aid;
	pjsua_acc_id			user_aid;
	// used to send message
	pj_str_t		to; 
	pj_str_t		mime_type; 
	pj_str_t		content;
	bShutdown = 0;
	pthread_cond_init(&_cond, NULL);
	pthread_mutex_init(&_mutex, NULL);

   if ( signal( SIGINT, signalHandler ) == SIG_ERR )
   {
      exit( -1 );
   }

   if ( signal( SIGTERM, signalHandler ) == SIG_ERR )
   {
      exit( -1 );
   }


	/**
	 * create pjsua first 
	 */
	status = pjsua_create();
	if(status != PJ_SUCCESS) {
		pjsua_perror(THIS_FILE, "Error initializing pjsua", status);
		return status;
	}
	/**
	 * init pjsua framework 
	 */
	/* configuration of ua */
	pjsua_config_default(&(ua_cfg));
	/**
	 * init callback
	 * set callback functions you need
	 * all the prototype of call are declared in pjsua.h
	 */
//	/* call related handle */
//	ua_cfg.cb.on_call_state					  = on_call_state;
//	ua_cfg.cb.on_incoming_call                = on_incoming_call;
//	ua_cfg.cb.on_call_tsx_state               = on_call_tsx_state;
//	ua_cfg.cb.on_call_media_state             = on_call_media_state;
//	ua_cfg.cb.on_call_sdp_created             = on_call_sdp_created;
//	ua_cfg.cb.on_stream_created               = on_stream_created;
//	ua_cfg.cb.on_stream_destroyed             = on_stream_destroyed;
//	ua_cfg.cb.on_dtmf_digit                   = on_dtmf_digit;
//	ua_cfg.cb.on_call_transfer_request        = on_call_transfer_request;
//	ua_cfg.cb.on_call_transfer_request2       = on_call_transfer_request2;
//	ua_cfg.cb.on_call_transfer_status         = on_call_transfer_status;
//	ua_cfg.cb.on_call_replace_request         = on_call_replace_request;
//	ua_cfg.cb.on_call_replace_request2        = on_call_replace_request2;
//	ua_cfg.cb.on_call_replaced                = on_call_replaced;
//	ua_cfg.cb.on_call_rx_offer                = on_call_rx_offer;
//	/* register handle */
	ua_cfg.cb.on_reg_started                  = on_reg_started;
	ua_cfg.cb.on_reg_state                    = on_reg_state;
	ua_cfg.cb.on_reg_state2                   = on_reg_state2;
//	/* subscirbe handle */
//	ua_cfg.cb.on_incoming_subscribe           = on_incoming_subscribe;
//	ua_cfg.cb.on_srv_subscribe_state          = on_srv_subscribe_state;
//	ua_cfg.cb.on_buddy_state                  = on_buddy_state;
//	ua_cfg.cb.on_buddy_evsub_state            = on_buddy_evsub_state;
//	/* message handle */
//	ua_cfg.cb.on_pager                        = on_pager;
	ua_cfg.cb.on_pager2                       = on_pager2;
//	ua_cfg.cb.on_pager_status                 = on_pager_status;
//	ua_cfg.cb.on_pager_status2                = on_pager_status2;
//	ua_cfg.cb.on_typing                       = on_typing;
//	ua_cfg.cb.on_typing2                      = on_typing2;
//	/* network handle */
//	ua_cfg.cb.on_nat_detect                   = on_nat_detect;
	ua_cfg.cb.on_call_redirected              = on_call_redirected;
//	ua_cfg.cb.on_mwi_state                    = on_mwi_state;
//	ua_cfg.cb.on_mwi_info                     = on_mwi_info;
//	ua_cfg.cb.on_transport_state              = on_transport_state;
//	ua_cfg.cb.on_call_media_transport_state   = on_call_media_transport_state;
//	ua_cfg.cb.on_ice_transport_error          = on_ice_transport_error;
//	ua_cfg.cb.on_snd_dev_operation            = on_snd_dev_operation;
//	/* media module handle */
//	ua_cfg.cb.on_call_media_event             = on_call_media_event;
//	ua_cfg.cb.on_create_media_transport       = on_create_media_transport;
	/* configure logging */
	pjsua_logging_config_default(&(log_cfg));
	log_cfg.msg_logging = PJ_FALSE;
	/* configure media */
	pjsua_media_config_default(&(media_cfg));
	status = pjsua_init(&(ua_cfg), 
			&(log_cfg), 
			NULL);
//			&(media_cfg));
	if(status != PJ_SUCCESS) {
		pjsua_perror(THIS_FILE, "Error initializing pjsua", status);
		return status;
	}

	status = pjsua_start();
	if(status != PJ_SUCCESS) {
		pjsua_perror(THIS_FILE, "Error starting pjsua", status);
		return status;
	}

	/**
	 * Setup transport
	 */
	pjsua_transport_config_default(&udp_cfg);
	udp_cfg.port = 5091;
	status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &udp_cfg, &transport_id);
	if(status != PJ_SUCCESS)
		return -1;

	/**
	 * add local account (sip:domain) which handle sip messages that don't matches any
	 * other accounts
	 */
	pjsua_acc_add_local(transport_id, PJ_TRUE, &local_aid);

	/**
	 * add user account and set it to default account 
	 */
	pjsua_acc_config_default(&acc_cfg);
	acc_cfg.id = pj_str("sip:8888@192.168.229.142");
	acc_cfg.reg_uri = pj_str("sip:192.168.229.142");
	acc_cfg.reg_timeout = 60;
	acc_cfg.cred_count = 1;
	acc_cfg.cred_info[0].realm = pj_str("192.168.229.142");
	acc_cfg.cred_info[0].scheme = pj_str("digest");
	acc_cfg.cred_info[0].username = pj_str("8888");
	acc_cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
	acc_cfg.cred_info[0].data = pj_str("SipPwd");
	acc_cfg.proxy_cnt = 1;
	acc_cfg.proxy[0] = pj_str("sip:192.168.229.142;lr");

	/**
	 * register_on_acc_add is true by default, add:
	 *
	 *		acc_cfg.register_on_acc_add = PJ_FALSE;
	 *
	 * to disable register on add.add later you can call the following 
	 * code to send a REGISTER.
	 *
	 *		pjsua_acc_set_registration(id_of_the_account, PJ_TRUE);
	 *
	 */
	status = pjsua_acc_add(&acc_cfg, PJ_TRUE, &user_aid);
	sleep(2);
	/**
	 * unregister account before exit
	 */


	to = pj_str("sip:Server_301007@192.168.1.197");
	to = pj_str("sip:Manage@192.168.1.197");
	mime_type = pj_str("text/plain");
	content = pj_str("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<SIP_XML EventType=\"TimeSync\">"
			"<Item operation=\"query\" time=\"0-0-0 0:0:0\"/>"
			"</SIP_XML>");
	content = pj_str("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<SIP_XML EventType=\"MediaPlay\">"
			"<User uri=\"sip:user1@192.168.1.197\"/>"
			"<Media type=\"1\" id=\"000001-001\"/>"
			"</SIP_XML>");
	content = pj_str("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<SIP_XML EventType=\"MediaTeardown\">"
			"<User uri=\"sip:user1@192.168.1.197\"/>"
			"<Media type=\"1\" id=\"000001-001\"/>"
			"</SIP_XML>");
	{
		pthread_t tid;
//		pthread_create(&tid, NULL, test_thread, &user_aid);
	}
//	pjsua_im_send(user_aid, &to, &mime_type, &content, NULL, NULL);
	sleep(1);
	printf("*******************************\n");
	printf("pjsua_im_send ok @ main\n");
	printf("*******************************\n");
//	pthread_mutex_lock(&_mutex);
//	pthread_cond_signal(&_cond);
//	pthread_mutex_unlock(&_mutex);
//	pjsua_call_setting setting;
//	pjsua_call_setting_default(&setting);
//	setting.aud_cnt = 0;
//	setting.vid_cnt = 0;
//	pjsua_call_id cid;
//	pjsua_msg_data msg_data;
//	pjsua_msg_data_init(&msg_data);
//	msg_data.content_type = pj_str("application/sdp");
//	msg_data.msg_body = pj_str(
//			"v=0\r\n"
//			"o=adapter 0 0 IN IP4 0.0.0.0\r\n"
//			"s=-\r\n"
//			"c=IN IP4 192.168.1.147\r\n"
//			"t=1371053400 1371057000\r\n"
//			"m=video 6000 TCP 96\r\n"
//			"a=rtpmap:98 L16/16000/2\r\n"
//			"a=ResID:301002-0005\r\n"
//			"a=WndIndex:1\r\n"
//			"a=recvonly\r\n");
//	sleep(2);
//	if(pjsua_call_make_call(user_aid, &to, &setting, NULL, NULL, &cid) != PJ_SUCCESS)
//		PJ_LOG(1, (__FILE__, "failed to make call\n"));
	while(!bShutdown)
	{
		sleep(1);
	}
//	pjsua_acc_set_registration(user_aid, PJ_FALSE);
	sleep(1);
	return status;

}
