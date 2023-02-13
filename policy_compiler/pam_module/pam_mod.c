#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <stdlib.h>

static struct pam_conv conv = {
        misc_conv,
        NULL
};

int main() {

}
