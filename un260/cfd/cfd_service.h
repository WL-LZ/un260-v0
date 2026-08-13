#ifndef CFD_SERVICE_H
#define CFD_SERVICE_H

#include <stdbool.h>

bool cfd_service_request_query(const char currency[4]);
void cfd_service_cancel_query(void);
bool cfd_service_take_query_result(const char currency[4]);

#endif
