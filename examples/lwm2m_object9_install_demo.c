#include <stdio.h>
#include <string.h>

/*
 * Focused LwM2M Object 9 (Software Management) install workflow demo.
 *
 * This is a local/mock example that models key Object 9 resources:
 * - /9/0/2  Software Package   (represented as "package_data")
 * - /9/0/3  Software Package URI
 * - /9/0/4  Install (Executable)
 * - /9/0/7  Update State
 * - /9/0/9  Update Result
 */

typedef enum {
    UPDATE_STATE_INITIAL = 0,
    UPDATE_STATE_DOWNLOAD_STARTED = 1,
    UPDATE_STATE_DOWNLOADED = 2,
    UPDATE_STATE_DELIVERED = 3,
    UPDATE_STATE_INSTALLED = 4
} update_state_t;

typedef enum {
    UPDATE_RESULT_INITIAL = 0,
    UPDATE_RESULT_DOWNLOADING = 1,
    UPDATE_RESULT_INSTALLED = 2,
    UPDATE_RESULT_NOT_ENOUGH_SPACE = 50,
    UPDATE_RESULT_INTEGRITY_FAILURE = 51,
    UPDATE_RESULT_UNSUPPORTED_PACKAGE_TYPE = 56,
    UPDATE_RESULT_INVALID_URI = 57,
    UPDATE_RESULT_INSTALL_FAILURE = 58
} update_result_t;

#define PACKAGE_URI_MAX_SIZE 256
#define PACKAGE_DATA_MAX_SIZE 320
#define PACKAGE_DATA_PREFIX "downloaded-bytes-from:"

typedef struct {
    char package_uri[PACKAGE_URI_MAX_SIZE];
    char package_data[PACKAGE_DATA_MAX_SIZE];
    update_state_t update_state;
    update_result_t update_result;
} object9_instance_t;

static const char *state_to_string(update_state_t state)
{
    switch (state) {
    case UPDATE_STATE_INITIAL:
        return "INITIAL";
    case UPDATE_STATE_DOWNLOAD_STARTED:
        return "DOWNLOAD_STARTED";
    case UPDATE_STATE_DOWNLOADED:
        return "DOWNLOADED";
    case UPDATE_STATE_DELIVERED:
        return "DELIVERED";
    case UPDATE_STATE_INSTALLED:
        return "INSTALLED";
    default:
        return "UNKNOWN_STATE";
    }
}

static const char *result_to_string(update_result_t result)
{
    switch (result) {
    case UPDATE_RESULT_INITIAL:
        return "INITIAL";
    case UPDATE_RESULT_DOWNLOADING:
        return "DOWNLOADING";
    case UPDATE_RESULT_INSTALLED:
        return "INSTALLED";
    case UPDATE_RESULT_NOT_ENOUGH_SPACE:
        return "NOT_ENOUGH_SPACE";
    case UPDATE_RESULT_INTEGRITY_FAILURE:
        return "INTEGRITY_FAILURE";
    case UPDATE_RESULT_UNSUPPORTED_PACKAGE_TYPE:
        return "UNSUPPORTED_PACKAGE_TYPE";
    case UPDATE_RESULT_INVALID_URI:
        return "INVALID_URI";
    case UPDATE_RESULT_INSTALL_FAILURE:
        return "INSTALL_FAILURE";
    default:
        return "UNKNOWN_RESULT";
    }
}

static void print_status(const object9_instance_t *obj9, const char *step)
{
    printf("[%s] Update State=%s (%d), Update Result=%s (%d)\n",
           step,
           state_to_string(obj9->update_state),
           obj9->update_state,
           result_to_string(obj9->update_result),
           obj9->update_result);
}

static int write_package_uri(object9_instance_t *obj9, const char *uri)
{
    if (uri == NULL ||
        (strncmp(uri, "http://", 7) != 0 && strncmp(uri, "https://", 8) != 0)) {
        obj9->update_result = UPDATE_RESULT_INVALID_URI;
        return -1;
    }

    snprintf(obj9->package_uri, sizeof(obj9->package_uri), "%s", uri);
    obj9->update_state = UPDATE_STATE_DOWNLOAD_STARTED;
    obj9->update_result = UPDATE_RESULT_DOWNLOADING;
    print_status(obj9, "download_started");

    /* Mock download completion */
    size_t prefix_len = sizeof(PACKAGE_DATA_PREFIX) - 1;
    size_t uri_len = strlen(obj9->package_uri);
    size_t required_size = prefix_len + uri_len + 1;

    if (required_size > sizeof(obj9->package_data)) {
        obj9->update_result = UPDATE_RESULT_NOT_ENOUGH_SPACE;
        return -1;
    }

    snprintf(obj9->package_data, sizeof(obj9->package_data),
             "%s%s", PACKAGE_DATA_PREFIX, obj9->package_uri);
    obj9->update_state = UPDATE_STATE_DOWNLOADED;
    obj9->update_result = UPDATE_RESULT_INITIAL;
    print_status(obj9, "download_completed");
    return 0;
}

static int execute_install(object9_instance_t *obj9)
{
    if (obj9->update_state != UPDATE_STATE_DOWNLOADED) {
        obj9->update_result = UPDATE_RESULT_INSTALL_FAILURE;
        return -1;
    }

    /* In a real client this is where package verification and installation occurs. */
    obj9->update_state = UPDATE_STATE_DELIVERED;

    /* Demo-only integrity check: real clients should verify checksum/signature. */
    if (strstr(obj9->package_data, PACKAGE_DATA_PREFIX) == NULL) {
        obj9->update_result = UPDATE_RESULT_INTEGRITY_FAILURE;
        return -1;
    }

    obj9->update_state = UPDATE_STATE_INSTALLED;
    obj9->update_result = UPDATE_RESULT_INSTALLED;
    return 0;
}

int main(void)
{
    object9_instance_t obj9 = {
        .update_state = UPDATE_STATE_INITIAL,
        .update_result = UPDATE_RESULT_INITIAL,
    };

    puts("LwM2M Object 9 software installation demo");
    print_status(&obj9, "initial");

    /* Server writes /9/0/3 (Software Package URI). */
    if (write_package_uri(&obj9, "http://example.org/apps/sample-app-v1.pkg") != 0) {
        print_status(&obj9, "write_package_uri_failed");
        return 1;
    }
    print_status(&obj9, "after_package_uri_write");

    /* Server executes /9/0/4 (Install). */
    if (execute_install(&obj9) != 0) {
        print_status(&obj9, "install_failed");
        return 1;
    }
    print_status(&obj9, "after_install_execute");

    puts("Demo complete: Object 9 instance transitioned to INSTALLED.");
    return 0;
}
