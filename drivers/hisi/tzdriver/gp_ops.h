#ifndef _GP_OPS_H_
#define _GP_OPS_H_

#ifndef CONFIG_BALONG_PLATFORM
extern struct ion_client *drm_ion_client;
#endif
int tc_user_param_valid(TC_NS_ClientContext *client_context, int n);
#ifdef CONFIG_BALONG_PLATFORM
int tc_client_call(TC_NS_ClientContext *context, TC_NS_DEV_File *dev_file,
		   uint8_t flags, unsigned int timeout);
#else
int tc_client_call(TC_NS_ClientContext *context, TC_NS_DEV_File *dev_file,
		   uint8_t flags);
#endif

#endif
