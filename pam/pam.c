#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <stdlib.h>

static struct pam_conv conv = {
	misc_conv,
	NULL
};

int main () {
	pam_handle_t *handle = NULL;
	const char *service_name = "../../../../../ect/pam.d/pam_example";
	int retval;
	char *username; // To be set by pam_get_item

	retval = pam_start(service_name, NULL, &conv, &handle);
	if(retval != PAM_SUCCESS) {
		fprintf(stderr, "Failure in pam initialization: %s\n", pam_strerror(handle, retval));
		return 1;
	}
	
	retval = pam_authenticate(handle, 0); // prompt user for username and pasword
	if(retval != PAM_SUCCESS) {
		fprintf(stderr, "Failure in pam authentication: %s\n", pam_strerror(handle, retval));
		return 1;
	}

	retval = pam_acct_mgmt(handle, 0); // Check that account can access the system
	if(retval != PAM_SUCCESS) {
		fprintf(stderr, "Failure in pam account management: %s\n", pam_strerror(handle, retval));
		return 1;
	}

	pam_get_item(handle, PAM_USER, (const void **)&username);
	printf("WELCOME, %s\n", username);

	printf("Do you want to change your password? (answer y/n): ");
	char answer = getc(stdin);
	if(answer == 'y') {
		retval = pam_chauthtok(handle, 0);
		if(retval != PAM_SUCCESS) {
			fprintf(stderr, "Failure in pam password: %s", pam_strerror(handle, retval));
			return 1;
		}
	}
	pam_end(handle, retval); // Terminate PAM transaction

}
