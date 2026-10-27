#ifndef SUPPLIER_H
#define SUPPLIER_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CONTACT_LEN 64
#define MAX_PHONE_LEN 32
#define MAX_EMAIL_LEN 64

typedef struct {
    int id;
    char name[MAX_CONTACT_LEN];
    char contact_person[MAX_CONTACT_LEN];
    char phone[MAX_PHONE_LEN];
    char email[MAX_EMAIL_LEN];
} Supplier;

#ifdef __cplusplus
}
#endif

#endif