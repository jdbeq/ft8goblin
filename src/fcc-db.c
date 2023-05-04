/*
 * functions for looking up users from the FCC database (sqlite) locally
 *
 * These require you to update your database from time to time...
 *
 * Hopefully I'll get around to cleaning up the scripts for that and including them soon!
 */
#include "sql.h"
/* In the PUBACC_EN table we have the following fields:
 * record_type
 * unique_system_identifier
 * uls_file_number
 * ebf_number
 * call_sign
 * entity_type
 * licensee_id
 * entity_name
 * first_name
 * mi
 * last_name
 * suffix
 * phone
 * fax
 * email
 * street_address
 * city
 * state
 * zip_code
 * po_box
 * attention_line
 * sgin
 * frn
 * applicant_type_code
 * applicant_type_other
 * status_code
 * status_date
 * lic_category_code
 * linked_license_id
 * linked_callsign
 */
// SELECT call_sign, first_name, mi, last_name, suffix, street_address, city, state, zip_code, po_box, attention_line, frn, applicant_type_code, phone, fax, email FROM PUBACC_EN ORDER BY (callsign);
