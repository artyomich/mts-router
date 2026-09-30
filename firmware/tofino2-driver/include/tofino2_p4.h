/* SPDX-License-Identifier: GPL-2.0 */
/*
 * tofino2_p4.h - P4 Pipeline Management Interface
 *
 * MTS-CR-9000 Core Router Tofino 2 Driver
 *
 * Copyright (c) 2024 MTS Router Project
 */

#ifndef TOFINO2_P4_H
#define TOFINO2_P4_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/radix-tree.h>

/* ============================================================================ */
/* P4 Program constants                                                         */
/* ============================================================================ */

#define P4_MAX_PROGRAM_SIZE    (4 * 1024 * 1024)  /* 4MB max program */
#define P4_MAX_TABLES          4096
#define P4_MAX_ACTIONS         2048
#define P4_MAX_EXTERN_OBJECTS  512
#define P4_MAX_PARSER_STATES   1024
#define P4_MAX_DEPARSER_OUTPUT 64
#define P4_MAX_CONTROL_FLOW    256
#define P4_MAX_CONSTANTS       256
#define P4_MAX_HEADERS         128
#define P4_MAX_VARIABLES       64
#define P4_MAX_DEFAULT_ACTIONS 32
#define P4_MAX_MATCH_KINDS     8
#define P4_TABLE_NAME_MAXLEN   64
#define P4_ACTION_NAME_MAXLEN  64
#define P4_HEADER_NAME_MAXLEN  64
#define P4_PARSER_STATE_NAME   64

/* ============================================================================ */
/* P4 program structure definitions                                             */
/* ============================================================================ */

/**
 * enum p4_table_match_type - P4 table match types
 */
enum p4_table_match_type {
    P4_MATCH_EXACT      = 0,
    P4_MATCH_LPM        = 1,
    P4_MATCH_TERNARY    = 2,
    P4_MATCH_RANGE      = 3,
    P4_MATCH_OPTIONAL   = 4,
    P4_MATCH_CONSTRUCTOR  = 5
};

/**
 * enum p4_action_type - P4 action primitive types
 */
enum p4_action_type {
    P4_ACTION_ASSIGN       = 0,
    P4_ACTION_CUSTOM       = 1,
    P4_ACTION_EXECUTE      = 2,
    P4_ACTION_JUMP         = 3,
    P4_ACTION_RETURN       = 4,
    P4_ACTION_ADD          = 5,
    P4_ACTION_SUB          = 6,
    P4_ACTION_MUL          = 7,
    P4_ACTION_DIV          = 8,
    P4_ACTION_MOD          = 9,
    P4_ACTION_AND          = 10,
    P4_ACTION_OR           = 11,
    P4_ACTION_XOR          = 12,
    P4_ACTION_NOT          = 13,
    P4_ACTION_SHIFT_LEFT   = 14,
    P4_ACTION_SHIFT_RIGHT  = 15,
    P4_ACTION_CONCAT       = 16,
    P4_ACTION_SUBSTRING    = 17,
    P4_ACTION_PARSE        = 18,
    P4_ACTION_DEPARSE      = 19,
    P4_ACTION_VERIFY       = 20,
    P4_ACTION_MODIFY_FIELD = 21,
    P4_ACTION_MOVE         = 22,
    P4_ACTION_PUSH         = 23,
    P4_ACTION_POP          = 24,
    P4_ACTION_DELETE       = 25,
    P4_ACTION_INSERT       = 26,
    P4_ACTION_RANDOM       = 27,
    P4_ACTION_HASH         = 28,
    P4_ACTION_CRC          = 29,
    P4_ACTION_checksum     = 30,
    P4_ACTION_METER       = 31,
    P4_ACTION_COUNTER      = 32,
    P4_ACTION_REGISTER     = 33,
    P4_ACTION_ADM          = 34,
    P4_ACTION_NOP          = 255
};

/**
 * enum p4_header_type - P4 header type classification
 */
enum p4_header_type {
    P4_HEADER_UNKNOWN,
    P4_HEADER_STANDARD,
    P4_HEADER_CUSTOM,
    P4_HEADER_ARRAY,
    P4_HEADER_METADATA
};

/**
 * struct p4_field - P4 field definition
 */
struct p4_field {
    char name[64];
    uint32_t bit_offset;
    uint32_t bit_width;
    uint32_t header_id;
    uint32_t is_signed;
    uint32_t is_const;
};

/**
 * struct p4_table - P4 table program element
 */
struct p4_table {
    char name[P4_TABLE_NAME_MAXLEN];
    uint32_t id;
    enum p4_table_match_type match_type;
    uint32_t key_fields;
    struct p4_field *key_field_list;
    uint32_t action_id;
    char action_name[P4_ACTION_NAME_MAXLEN];
    uint32_t max_size;
    uint32_t implementation;
    uint32_t counters;
    uint32_t meters;
    uint32_t default_action_id;
    char default_action_name[P4_ACTION_NAME_MAXLEN];
    uint32_t const_tables;
    uint8_t *const_table_data;
    uint32_t size_optimization;
    uint32_t preallocation;
    uint32_t auto_preallocate;
    uint32_t extern_objects;
    uint32_t *extern_object_ids;
};

/**
 * struct p4_action - P4 action program element
 */
struct p4_action {
    char name[P4_ACTION_NAME_MAXLEN];
    uint32_t id;
    uint32_t params;
    struct p4_field *params_list;
    uint32_t primitives;
    struct p4_action_primitive *primitives_list;
    uint32_t control_flow;
    uint32_t *control_flow_targets;
    uint32_t locals;
    struct p4_field *locals_list;
    uint32_t depth;
    uint32_t complexity;
};

/**
 * struct p4_action_primitive - Single P4 action primitive
 */
struct p4_action_primitive {
    enum p4_action_type type;
    uint32_t target_field;
    uint32_t source_field;
    uint32_t immediate_value;
    uint32_t param_index;
    uint32_t extra_data;
};

/**
 * struct p4_header - P4 header definition
 */
struct p4_header {
    char name[P4_HEADER_NAME_MAXLEN];
    uint32_t id;
    enum p4_header_type type;
    uint32_t fields;
    struct p4_field *fields_list;
    uint32_t valid_field;
    uint32_t first_bit;
    uint32_t last_bit;
    uint32_t size;
    uint32_t is_optional;
    uint32_t is_const;
};

/**
 * struct p4_parser_state - P4 parser state definition
 */
struct p4_parser_state {
    char name[P4_PARSER_STATE_NAME];
    uint32_t id;
    uint32_t type;       /* EXTRACT, SELECT, DEPARSER */
    uint32_t next_state; /* Default next state */
    uint32_t extracted_fields;
    struct p4_field **extracted_fields_list;
    uint32_t select_cases;
    uint32_t *select_targets;
    uint32_t select_fields;
    struct p4_field **select_fields_list;
};

/**
 * struct p4_extern_object - P4 extern object instance
 */
struct p4_extern_object {
    char name[64];
    uint32_t id;
    uint32_t type;      /* METER, COUNTER, REGISTER, HASH, CRC */
    uint32_t params;
    uint32_t *params_list;
    uint32_t is_const;
};

/**
 * struct p4_program - Complete P4 program
 */
struct p4_program {
    char name[64];
    uint32_t version_major;
    uint32_t version_minor;
    uint32_t checksum;
    uint32_t tables;
    struct p4_table *tables_list;
    uint32_t actions;
    struct p4_action *actions_list;
    uint32_t headers;
    struct p4_header *headers_list;
    uint32_t parser_states;
    struct p4_parser_state *parser_states_list;
    uint32_t extern_objects;
    struct p4_extern_object *extern_objects_list;
    uint32_t metadata;
    uint32_t deparser_output;
    uint32_t constants;
    uint32_t *constants_list;
    uint32_t control_flow_depth;
    uint32_t complexity_score;
    uint32_t is_compiled;
    uint32_t is_loaded;
    uint8_t *compiled_code;
    uint32_t compiled_size;
    uint8_t *binary_blob;
    uint32_t binary_size;
};

/* ============================================================================ */
/* Pipeline configuration                                                       */
/* ============================================================================ */

/**
 * struct p4_pipeline_config - P4 pipeline configuration
 */
struct p4_pipeline_config {
    uint32_t stage;                        /* Pipeline stage */
    uint32_t tables;                       /* Tables in stage */
    uint32_t *table_ids;                   /* Table ID array */
    uint32_t parallel;                     /* Parallel execution */
    uint32_t multicast_groups;             /* Multicast group count */
    uint32_t *multicast_groups_ids;        /* Multicast group IDs */
    uint32_t resource_limits;              /* Resource limits */
    uint32_t max_entries_per_table;        /* Max entries per table */
    uint32_t preallocation_pct;            /* Preallocation percentage */
    uint32_t size_optimize;                /* Size optimization */
    uint32_t hash_algorithm;               /* Hash algorithm type */
    uint32_t hash_seed;                    /* Hash seed */
    uint32_t meter_config;                 /* Meter configuration */
    uint32_t counter_config;               /* Counter configuration */
};

/* ============================================================================ */
/* P4 compilation results                                                       */
/* ============================================================================ */

/**
 * struct p4_compile_result - P4 compilation result
 */
struct p4_compile_result {
    uint32_t status;                        /* 0 = success */
    uint32_t errors;                        /* Error count */
    uint32_t warnings;                      /* Warning count */
    char error_msg[256];                    /* Error message */
    uint32_t tables_allocated;              /* Tables allocated */
    uint32_t actions_allocated;             /* Actions allocated */
    uint32_t externs_allocated;             /* Externs allocated */
    uint32_t resources_used;                /* Resources used */
    uint32_t resources_total;               /* Total resources */
    uint32_t pipeline_depth;                /* Pipeline depth */
    uint32_t critical_path;                 /* Critical path latency */
    uint32_t memory_used;                   /* Memory used (bytes) */
    uint32_t memory_total;                  /* Total memory (bytes) */
};

/* ============================================================================ */
/* API Functions                                                                  */
/* ============================================================================ */

/* P4 program loading */
struct p4_program *p4_program_load(const char *name,
                                    const uint8_t *data,
                                    uint32_t size);
int p4_program_unload(struct p4_program *prog);
int p4_program_clone(struct p4_program *src, struct p4_program **dst);
int p4_program_validate(struct p4_program *prog, char *err, uint32_t err_size);
uint32_t p4_program_checksum(struct p4_program *prog);

/* P4 compilation */
int p4_program_compile(struct p4_program *prog,
                        struct p4_compile_result *result);
int p4_program_verify(struct p4_program *prog);
int p4_program_dump(struct p4_program *prog, char *buf, uint32_t buf_size);

/* Table management */
int p4_table_create(struct p4_program *prog, struct p4_table *tbl);
int p4_table_destroy(struct p4_program *prog, uint32_t table_id);
struct p4_table *p4_table_find(struct p4_program *prog, const char *name);
int p4_table_get_stats(struct p4_table *tbl, uint32_t *hits,
                        uint32_t *misses, uint32_t *used,
                        uint32_t *max);

/* Action management */
int p4_action_create(struct p4_program *prog, struct p4_action *act);
int p4_action_destroy(struct p4_program *prog, uint32_t action_id);
struct p4_action *p4_action_find(struct p4_program *prog, const char *name);

/* Header management */
struct p4_header *p4_header_find(struct p4_program *prog, const char *name);
int p4_header_validate(struct p4_header *hdr);

/* Parser state management */
struct p4_parser_state *p4_parser_state_find(struct p4_program *prog,
                                              const char *name);
int p4_parser_validate(struct p4_program *prog);

/* Extern object management */
int p4_extern_create(struct p4_program *prog, struct p4_extern_object *ext);
int p4_extern_destroy(struct p4_program *prog, uint32_t extern_id);
struct p4_extern_object *p4_extern_find(struct p4_program *prog,
                                         uint32_t extern_id);

/* Pipeline management */
int p4_pipeline_configure(struct p4_pipeline_config *config);
int p4_pipeline_load(struct p4_program *prog, uint32_t pipeline_id);
int p4_pipeline_unload(uint32_t pipeline_id);
int p4_pipeline_get_config(uint32_t pipeline_id,
                            struct p4_pipeline_config *config);

/* Resource management */
int p4_resources_check(struct p4_program *prog,
                        struct p4_compile_result *result);
int p4_resources_allocate(struct p4_program *prog);
void p4_resources_release(struct p4_program *prog);
uint32_t p4_resources_usage(void);
uint32_t p4_resources_free(void);

#endif /* TOFINO2_P4_H */
