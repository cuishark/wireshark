
#define CAN_SET_CAPTURE_BUFFER_SIZE 1
#define HAVE_ARPA_INET_H 1
#define HAVE_ARPA_NAMESER_H 1
#define HAVE_BPF_IMAGE 1
#define HAVE_CXX11 1
#define HAVE_DLFCN_H 1
#define HAVE_FCNTL_H 1
#define HAVE_FLOORL 1
#define HAVE_GETIFADDRS 1
#define HAVE_GETOPT_H 1
#define HAVE_GETOPT_LONG 1
#define HAVE_GRP_H 1
#define HAVE_IFADDRS_H 1
#define HAVE_INFLATEPRIME 1
#define HAVE_INTTYPES_H 1
#define HAVE_LIBPCAP 1
#define HAVE_LIBZ 1
#define HAVE_LINUX_IF_BONDING_H 1
#define HAVE_LINUX_SOCKIOS_H 1
#define HAVE_LRINT 1
#define HAVE_MEMORY_H 1
#define HAVE_MKDTEMP 1
#define HAVE_MKSTEMPS 1
#define HAVE_NETDB_H 1
#define HAVE_NETINET_IN_H 1
#define HAVE_NL80211 1
#define HAVE_NL80211_CMD_SET_CHANNEL 1
#define HAVE_NL80211_SPLIT_WIPHY_DUMP 1
#define HAVE_NL80211_VHT_CAPABILITY 1
#define HAVE_PCAP_BREAKLOOP 1
#define HAVE_PCAP_CREATE 1
#define HAVE_PCAP_DATALINK_NAME_TO_VAL 1
#define HAVE_PCAP_DATALINK_VAL_TO_DESCRIPTION 1
#define HAVE_PCAP_DATALINK_VAL_TO_NAME 1
#define HAVE_PCAP_FINDALLDEVS 1
#define HAVE_PCAP_FREECODE 1
#define HAVE_PCAP_FREE_DATALINKS 1
#define HAVE_PCAP_GET_SELECTABLE_FD 1
#define HAVE_PCAP_LIB_VERSION 1
#define HAVE_PCAP_LIST_DATALINKS 1
#define HAVE_PCAP_OPEN_DEAD 1
#define HAVE_PCAP_SET_DATALINK 1
#define HAVE_PCAP_SET_TSTAMP_PRECISION 1
#define HAVE_PCAP_SET_TSTAMP_TYPE 1
#define HAVE_PLUGINS 1
#define HAVE_PWD_H 1
#define HAVE_SETRESGID 1
#define HAVE_SETRESUID 1
#define HAVE_SSE4_2 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_STRPTIME 1
#define HAVE_STRUCT_TM_TM_ZONE 1
#define HAVE_SYSCONF 1
#define HAVE_SYS_IOCTL_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_SELECT_H 1
#define HAVE_SYS_SOCKET_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_UTSNAME_H 1
#define HAVE_SYS_WAIT_H 1
#define HAVE_TM_ZONE 1
#define HAVE_UNISTD_H 1
#define HAVE_ZLIB 1
#define LT_OBJDIR ".libs/"
#define PCAP_NG_DEFAULT 1
#define STDC_HEADERS 1


#include "file.h"

#include <ws_diag_control.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>

#include <getopt.h>
#include "wsutil/wsgetopt.h"
#include <glib.h>
#include <epan/exceptions.h>
#include <epan/epan.h>

#include <wsutil/clopts_common.h>
#include <wsutil/cmdarg_err.h>
#include <wsutil/crash_info.h>
#include <wsutil/filesystem.h>
#include <wsutil/file_util.h>
#include <wsutil/privileges.h>
#include <wsutil/report_message.h>
#include <version_info.h>
#include <wiretap/wtap_opttypes.h>
#include <wiretap/pcapng.h>

#include <epan/timestamp.h>

#include <epan/timestamp.h>
#include <epan/packet.h>
#include "frame_tvbuff.h"
#include <epan/disabled_protos.h>
#include <epan/prefs.h>
#include <epan/column.h>
#include <epan/decode_as.h>
#include <epan/print.h>
#include <epan/addr_resolv.h>
#include "ui/taps.h"
#include "ui/util.h"
#include "ui/ws_ui_util.h"
#include "ui/decode_as_utils.h"
#include "ui/filter_files.h"
#include "ui/cli/tshark-tap.h"
#include "ui/cli/tap-exportobject.h"
#include "ui/tap_export_pdu.h"
#include "ui/dissect_opts.h"
#include "ui/failure_message.h"
#include "epan/register.h"
#include <epan/epan_dissect.h>
#include <epan/tap.h>
#include <epan/stat_tap_ui.h>
#include <epan/conversation_table.h>
#include <epan/srt_table.h>
#include <epan/rtd_table.h>
#include <epan/ex-opt.h>
#include <epan/exported_pdu.h>

#include "capture_opts.h"
#include "caputils/capture-pcap-util.h"
#include "caputils/capture_ifinfo.h"
#include <capchild/capture_session.h>
#include <capchild/capture_sync.h>
#include <capture_info.h>
#include "log.h"
#include <epan/funnel.h>

#include <wsutil/str_util.h>
#include <wsutil/utf8_entities.h>

#include "extcap.h"

#ifdef HAVE_PLUGINS
#include <wsutil/plugins.h>
#endif

/* Exit codes */
#define INVALID_OPTION 1
#define INVALID_INTERFACE 2
#define INVALID_FILE 2
#define INVALID_FILTER 2
#define INVALID_EXPORT 2
#define INVALID_CAPABILITY 2
#define INVALID_TAP 2
#define INVALID_DATA_LINK 2
#define INVALID_TIMESTAMP_TYPE 2
#define INVALID_CAPTURE 2
#define INIT_FAILED 2

/*
 * values 128..65535 are capture+dissect options, 65536 is used by
 * ui/commandline.c, so start tshark-specific options 1000 after this
 */
#define LONGOPT_COLOR (65536+1000)
#define LONGOPT_NO_DUPLICATE_KEYS (65536+1001)

#if 0
#define tshark_debug(...) g_warning(__VA_ARGS__)
#else
#define tshark_debug(...)
#endif

capture_file cfile;

static guint32 cum_bytes;
static frame_data ref_frame;
static frame_data prev_dis_frame;
static frame_data prev_cap_frame;

static gboolean perform_two_pass_analysis;
static guint32 epan_auto_reset_count = 0;
static gboolean epan_auto_reset = FALSE;

/*
 * The way the packet decode is to be written.
 */
typedef enum {
  WRITE_TEXT,   /* summary or detail text */
  WRITE_XML,    /* PDML or PSML */
  WRITE_FIELDS, /* User defined list of fields */
  WRITE_JSON,   /* JSON */
  WRITE_JSON_RAW,   /* JSON only raw hex */
  WRITE_EK      /* JSON bulk insert to Elasticsearch */
  /* Add CSV and the like here */
} output_action_e;

static output_action_e output_action;
static gboolean do_dissection;     /* TRUE if we have to dissect each packet */
static gboolean print_packet_info; /* TRUE if we're to print packet information */
static gboolean print_summary;     /* TRUE if we're to print packet summary information */
static gboolean print_details;     /* TRUE if we're to print packet details information */
static gboolean print_hex;         /* TRUE if we're to print hex/ascci information */
static gboolean line_buffered;
static gboolean really_quiet = FALSE;
static gchar* delimiter_char = " ";
static gboolean dissect_color = FALSE;
static print_format_e print_format = PR_FMT_TEXT;
static print_stream_t *print_stream = NULL;
static output_fields_t* output_fields  = NULL;
static gchar **protocolfilter = NULL;
static pf_flags protocolfilter_flags = PF_NONE;
static gboolean no_duplicate_keys = FALSE;
static proto_node_children_grouper_func node_children_grouper = proto_node_group_children_by_unique;
static const char *separator = "";
static gboolean prefs_loaded = FALSE;
static gboolean print_packet_counts;
static capture_options global_capture_opts;
static capture_session global_capture_session;
static info_data_t global_info_data;
static gboolean capture(void);
static void report_counts(void);
static void capture_cleanup(int);

static void reset_epan_mem(capture_file *cf, epan_dissect_t *edt, gboolean tree, gboolean visual);
static gboolean process_cap_file(capture_file *, char *, int, gboolean, int, gint64);
static gboolean process_packet_single_pass(capture_file *cf,
    epan_dissect_t *edt, gint64 offset, wtap_rec *rec,
    const guchar *pd, guint tap_flags);
static void show_print_file_io_error(int err);
static gboolean write_preamble(capture_file *cf);
static gboolean print_packet(capture_file *cf, epan_dissect_t *edt);
static gboolean write_finale(void);

static void failure_warning_message(const char *msg_format, va_list ap);
static void open_failure_message(const char *filename, int err,
    gboolean for_writing);
static void read_failure_message(const char *filename, int err);
static void write_failure_message(const char *filename, int err);
static void failure_message_cont(const char *msg_format, va_list ap);

static GHashTable *output_only_tables = NULL;

struct string_elem {
  const char *sstr;   /* The short string */
  const char *lstr;   /* The long string */
};

static gint
string_compare(gconstpointer a, gconstpointer b)
{
  return strcmp(((const struct string_elem *)a)->sstr,
                ((const struct string_elem *)b)->sstr);
}

static void
string_elem_print(gpointer data, gpointer not_used _U_)
{
  fprintf(stderr, "    %s - %s\n",
          ((struct string_elem *)data)->sstr,
          ((struct string_elem *)data)->lstr);
}

static void
list_capture_types(void) {
  int                 i;
  struct string_elem *captypes;
  GSList             *list = NULL;

  captypes = g_new(struct string_elem, WTAP_NUM_FILE_TYPES_SUBTYPES);

  fprintf(stderr, "tshark: The available capture file types for the \"-F\" flag are:\n");
  for (i = 0; i < WTAP_NUM_FILE_TYPES_SUBTYPES; i++) {
    if (wtap_dump_can_open(i)) {
      captypes[i].sstr = wtap_file_type_subtype_short_string(i);
      captypes[i].lstr = wtap_file_type_subtype_string(i);
      list = g_slist_insert_sorted(list, &captypes[i], string_compare);
    }
  }
  g_slist_foreach(list, string_elem_print, NULL);
  g_slist_free(list);
  g_free(captypes);
}

static void
list_read_capture_types(void) {
  int                 i;
  struct string_elem *captypes;
  GSList             *list = NULL;
  const char *magic = "Magic-value-based";
  const char *heuristic = "Heuristics-based";

  /* this is a hack, but WTAP_NUM_FILE_TYPES_SUBTYPES is always >= number of open routines so we're safe */
  captypes = g_new(struct string_elem, WTAP_NUM_FILE_TYPES_SUBTYPES);

  fprintf(stderr, "tshark: The available read file types for the \"-X read_format:\" option are:\n");
  for (i = 0; open_routines[i].name != NULL; i++) {
    captypes[i].sstr = open_routines[i].name;
    captypes[i].lstr = (open_routines[i].type == OPEN_INFO_MAGIC) ? magic : heuristic;
    list = g_slist_insert_sorted(list, &captypes[i], string_compare);
  }
  g_slist_foreach(list, string_elem_print, NULL);
  g_slist_free(list);
  g_free(captypes);
}

static void
print_current_user(void) {
  gchar *cur_user, *cur_group;

  if (started_with_special_privs()) {
    cur_user = get_cur_username();
    cur_group = get_cur_groupname();
    fprintf(stderr, "Running as user \"%s\" and group \"%s\".",
      cur_user, cur_group);
    g_free(cur_user);
    g_free(cur_group);
    if (running_with_special_privs()) {
      fprintf(stderr, " This could be dangerous.");
    }
    fprintf(stderr, "\n");
  }
}

static void
get_tshark_compiled_version_info(GString *str)
{
  get_compiled_caplibs_version(str);
}

static void
get_tshark_runtime_version_info(GString *str)
{
  epan_get_runtime_version_info(str);
}


static gboolean
must_do_dissection(dfilter_t *rfcode, dfilter_t *dfcode,
                   gchar *volatile pdu_export_arg)
{
  return print_packet_info || rfcode || dfcode || pdu_export_arg ||
      tap_listeners_require_dissection() || dissect_color;
}

int
main(int argc, char *argv[])
{
  GString             *comp_info_str;
  GString             *runtime_info_str;
  char                *init_progfile_dir_error;
  int                  opt;
  static const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'v'},
    LONGOPT_CAPTURE_COMMON
    LONGOPT_DISSECT_COMMON
    {"print", no_argument, NULL, 'P'},
    {"export-objects", required_argument, NULL, LONGOPT_EXPORT_OBJECTS},
    {"color", no_argument, NULL, LONGOPT_COLOR},
    {"no-duplicate-keys", no_argument, NULL, LONGOPT_NO_DUPLICATE_KEYS},
    {0, 0, 0, 0 }
  };
  gboolean             arg_error = FALSE;

  int                  err;
  volatile gboolean    success;
  volatile int         exit_status = EXIT_SUCCESS;
#ifdef HAVE_LIBPCAP
  int                  caps_queries = 0;
  gboolean             start_capture = FALSE;
  gchar               *err_str;
#else
  gboolean             capture_option_specified = FALSE;
#endif
  gboolean             quiet = FALSE;
#ifdef PCAP_NG_DEFAULT
  volatile int         out_file_type = WTAP_FILE_TYPE_SUBTYPE_PCAPNG;
#else
  volatile int         out_file_type = WTAP_FILE_TYPE_SUBTYPE_PCAP;
#endif
  volatile gboolean    out_file_name_res = FALSE;
  volatile int         in_file_type = WTAP_TYPE_AUTO;
  gchar               *volatile cf_name = NULL;
  gchar               *rfilter = NULL;
  gchar               *dfilter = NULL;
#ifdef HAVE_PCAP_OPEN_DEAD
  struct bpf_program   fcode;
#endif
  dfilter_t           *rfcode = NULL;
  dfilter_t           *dfcode = NULL;
  gchar               *err_msg;
  e_prefs             *prefs_p;
  /* int                  log_flags; */
  gchar               *output_only = NULL;
  gchar               *volatile pdu_export_arg = NULL;
  const char          *volatile exp_pdu_filename = NULL;
  exp_pdu_t            exp_pdu_tap_data;

#define OPTSTRING "+2" OPTSTRING_CAPTURE_COMMON OPTSTRING_DISSECT_COMMON "M:C:e:E:F:gG:hH:j:J:lo:O:PqQr:R:S:T:U:vVw:W:xX:Y:z:"

  static const char    optstring[] = OPTSTRING;

  tshark_debug("tshark started with %d args", argc);

  /* Set the C-language locale to the native environment. */
  setlocale(LC_ALL, "");

  cmdarg_err_init(failure_warning_message, failure_message_cont);

  /*
   * Get credential information for later use, and drop privileges
   * before doing anything else.
   * Let the user know if anything happened.
   */
  init_process_policies();
  relinquish_special_privs_perm();
  print_current_user();

  /*
   * Attempt to get the pathname of the directory containing the
   * executable file.
   */
  init_progfile_dir_error = init_progfile_dir(argv[0], main);
  if (init_progfile_dir_error != NULL) {
    fprintf(stderr,
            "tshark: Can't get pathname of directory containing the tshark program: %s.\n"
            "It won't be possible to capture traffic.\n"
            "Report this to the Wireshark developers.",
            init_progfile_dir_error);
    g_free(init_progfile_dir_error);
  }

  initialize_funnel_ops();

  /* Get the compile-time version information string */
  comp_info_str = get_compiled_version_info(get_tshark_compiled_version_info,
                                            epan_get_compiled_version_info);

  /* Get the run-time version information string */
  runtime_info_str = get_runtime_version_info(get_tshark_runtime_version_info);

  /* Add it to the information to be reported on a crash. */
  ws_add_crash_info("TShark (Wireshark) %s\n"
         "\n"
         "%s"
         "\n"
         "%s",
      get_ws_vcs_version_info(), comp_info_str->str, runtime_info_str->str);
  g_string_free(comp_info_str, TRUE);
  g_string_free(runtime_info_str, TRUE);

  opterr = 0;

  print_summary = TRUE;
  print_details = TRUE;
  print_packet_info = TRUE;
  print_hex = TRUE;

  init_report_message(failure_warning_message, failure_warning_message,
                      open_failure_message, read_failure_message,
                      write_failure_message);

#ifdef HAVE_LIBPCAP
  capture_opts_init(&global_capture_opts);
  capture_session_init(&global_capture_session, &cfile);
#endif

  timestamp_set_type(TS_RELATIVE);
  timestamp_set_precision(TS_PREC_AUTO);
  timestamp_set_seconds_type(TS_SECONDS_DEFAULT);
  wtap_init(TRUE);

  if (!epan_init(register_all_protocols,
        register_all_protocol_handoffs, NULL, NULL)) {
    exit_status = INIT_FAILED;
    goto clean_exit;
  }

  register_all_plugin_tap_listeners();
  extcap_register_preferences();
  /* Register all tap listeners. */
  for (tap_reg_t *t = tap_reg_listener; t->cb_func != NULL; t++) {
    t->cb_func();
  }
  conversation_table_set_gui_info(init_iousers);
  hostlist_table_set_gui_info(init_hostlists);
  srt_table_iterate_tables(register_srt_tables, NULL);
  rtd_table_iterate_tables(register_rtd_tables, NULL);
  stat_tap_iterate_tables(register_simple_stat_tables, NULL);

  tshark_debug("tshark reading settings");
  prefs_p = epan_load_settings();
  prefs_loaded = TRUE;
  read_filter_list(CFILTER_LIST);
  cap_file_init(&cfile);
  print_format = PR_FMT_TEXT;
  delimiter_char = " ";
  output_fields = output_fields_new();
  optind = 0;
  opterr = 1;

  /* Now get our args */
  while ((opt = getopt_long(argc, argv, optstring, long_options, NULL)) != -1) {
    switch (opt) {
    case 'i':        /* Use interface x */
      exit_status = capture_opts_add_opt(&global_capture_opts, opt, optarg, &start_capture);
      if (exit_status != 0) {
        goto clean_exit;
      }
      break;
    case 'r':        /* Read capture file x */
      cf_name = g_strdup(optarg);
      break;
    default:
    case '?':        /* Bad flag - print usage message */
      switch(optopt) {
      case 'F':
        list_capture_types();
        break;
      default:
        break;
      }
      exit_status = INVALID_OPTION;
      goto clean_exit;
      break;
    }
  }

  if (no_duplicate_keys && output_action != WRITE_JSON && output_action != WRITE_JSON_RAW) {
    cmdarg_err("--no-duplicate-keys can only be used with \"-T json\" and \"-T jsonraw\"");
    exit_status = INVALID_OPTION;
    goto clean_exit;
  }

  /* If we specified output fields, but not the output field type... */
  if ((WRITE_FIELDS != output_action
        && WRITE_XML != output_action
        && WRITE_JSON != output_action
        && WRITE_EK != output_action)
      && 0 != output_fields_num_fields(output_fields)) {
        cmdarg_err("Output fields were specified with \"-e\", "
            "but \"-Tek, -Tfields, -Tjson or -Tpdml\" was not specified.");
        exit_status = INVALID_OPTION;
        goto clean_exit;
  } else if (WRITE_FIELDS == output_action && 0 == output_fields_num_fields(output_fields)) {
        cmdarg_err("\"-Tfields\" was specified, but no fields were "
                    "specified with \"-e\".");

        exit_status = INVALID_OPTION;
        goto clean_exit;
  }

  if (dissect_color) {
    if (!color_filters_init(&err_msg, NULL)) {
      fprintf(stderr, "%s\n", err_msg);
      g_free(err_msg);
    }
  }

  /* If no capture filter or display filter has been specified, and there are
     still command-line arguments, treat them as the tokens of a capture
     filter (if no "-r" flag was specified) or a display filter (if a "-r"
     flag was specified. */
  if (optind < argc) {
    if (cf_name != NULL) {
      if (dfilter != NULL) {
        cmdarg_err("Display filters were specified both with \"-d\" "
            "and with additional command-line arguments.");
        exit_status = INVALID_OPTION;
        goto clean_exit;
      }
      dfilter = get_args_as_string(argc, argv, optind);
    } else {
#ifdef HAVE_LIBPCAP
      guint i;

      if (global_capture_opts.default_options.cfilter) {
        cmdarg_err("A default capture filter was specified both with \"-f\""
            " and with additional command-line arguments.");
        exit_status = INVALID_OPTION;
        goto clean_exit;
      }
      for (i = 0; i < global_capture_opts.ifaces->len; i++) {
        interface_options *interface_opts;
        interface_opts = &g_array_index(global_capture_opts.ifaces, interface_options, i);
        if (interface_opts->cfilter == NULL) {
          interface_opts->cfilter = get_args_as_string(argc, argv, optind);
        } else {
          cmdarg_err("A capture filter was specified both with \"-f\""
              " and with additional command-line arguments.");
          exit_status = INVALID_OPTION;
          goto clean_exit;
        }
      }
      global_capture_opts.default_options.cfilter = get_args_as_string(argc, argv, optind);
#else
      capture_option_specified = TRUE;
#endif
    }
  }

#ifdef HAVE_LIBPCAP
  if (!global_capture_opts.saving_to_file) {
    /* We're not saving the capture to a file; if "-q" wasn't specified,
       we should print packet information */
    if (!quiet)
      print_packet_info = TRUE;
  } else {
    /* We're saving to a file; if we're writing to the standard output.
       and we'll also be writing dissected packets to the standard
       output, reject the request.  At best, we could redirect that
       to the standard error; we *can't* write both to the standard
       output and have either of them be useful. */
    if (strcmp(global_capture_opts.save_file, "-") == 0 && print_packet_info) {
      cmdarg_err("You can't write both raw packet data and dissected packets"
          " to the standard output.");
      exit_status = INVALID_OPTION;
      goto clean_exit;
    }
  }
#else
  /* We're not saving the capture to a file; if "-q" wasn't specified,
     we should print packet information */
  if (!quiet)
    print_packet_info = TRUE;
#endif

#ifndef HAVE_LIBPCAP
  if (capture_option_specified)
    cmdarg_err("This version of TShark was not built with support for capturing packets.");
#endif
  if (arg_error) {
    exit_status = INVALID_OPTION;
    goto clean_exit;
  }

  if (print_hex) {
    if (output_action != WRITE_TEXT && output_action != WRITE_JSON && output_action != WRITE_JSON_RAW && output_action != WRITE_EK) {
      cmdarg_err("Raw packet hex data can only be printed as text, PostScript, JSON, JSONRAW or EK JSON");
      exit_status = INVALID_OPTION;
      goto clean_exit;
    }
  }

  if (output_only != NULL) {
    char *ps;

    if (!print_details) {
      cmdarg_err("-O requires -V");
      exit_status = INVALID_OPTION;
      goto clean_exit;
    }

    output_only_tables = g_hash_table_new (g_str_hash, g_str_equal);
    for (ps = strtok (output_only, ","); ps; ps = strtok (NULL, ",")) {
      g_hash_table_insert(output_only_tables, (gpointer)ps, (gpointer)ps);
    }
  }

  if (rfilter != NULL && !perform_two_pass_analysis) {
    cmdarg_err("-R without -2 is deprecated. For single-pass filtering use -Y.");
    exit_status = INVALID_OPTION;
    goto clean_exit;
  }

#ifdef HAVE_LIBPCAP
  if (caps_queries) {
    /* We're supposed to list the link-layer/timestamp types for an interface;
       did the user also specify a capture file to be read? */
    if (cf_name) {
      /* Yes - that's bogus. */
      cmdarg_err("You can't specify %s and a capture file to be read.",
                 caps_queries & CAPS_QUERY_LINK_TYPES ? "-L" : "--list-time-stamp-types");
      exit_status = INVALID_OPTION;
      goto clean_exit;
    }
    /* No - did they specify a ring buffer option? */
    if (global_capture_opts.multi_files_on) {
      cmdarg_err("Ring buffer requested, but a capture isn't being done.");
      exit_status = INVALID_OPTION;
      goto clean_exit;
    }
  } else {
    if (cf_name) {
      /*
       * "-r" was specified, so we're reading a capture file.
       * Capture options don't apply here.
       */

      /* We don't support capture filters when reading from a capture file
         (the BPF compiler doesn't support all link-layer types that we
         support in capture files we read). */
      if (global_capture_opts.default_options.cfilter) {
        cmdarg_err("Only read filters, not capture filters, "
          "can be specified when reading a capture file.");
        exit_status = INVALID_OPTION;
        goto clean_exit;
      }
      if (global_capture_opts.multi_files_on) {
        cmdarg_err("Multiple capture files requested, but "
                   "a capture isn't being done.");
        exit_status = INVALID_OPTION;
        goto clean_exit;
      }
      if (global_capture_opts.has_file_duration) {
        cmdarg_err("Switching capture files after a time period was specified, but "
                   "a capture isn't being done.");
        exit_status = INVALID_OPTION;
        goto clean_exit;
      }
      if (global_capture_opts.has_file_interval) {
        cmdarg_err("Switching capture files after a time interval was specified, but "
                   "a capture isn't being done.");
        exit_status = INVALID_OPTION;
        goto clean_exit;
      }
      if (global_capture_opts.has_ring_num_files) {
        cmdarg_err("A ring buffer of capture files was specified, but "
          "a capture isn't being done.");
        exit_status = INVALID_OPTION;
        goto clean_exit;
      }
      if (global_capture_opts.has_autostop_files) {
        cmdarg_err("A maximum number of capture files was specified, but "
          "a capture isn't being done.");
        exit_status = INVALID_OPTION;
        goto clean_exit;
      }
      if (global_capture_opts.capture_comment) {
        cmdarg_err("A capture comment was specified, but "
          "a capture isn't being done.\nThere's no support for adding "
          "a capture comment to an existing capture file.");
        exit_status = INVALID_OPTION;
        goto clean_exit;
      }

      /* Note: TShark now allows the restriction of a _read_ file by packet count
       * and byte count as well as a write file. Other autostop options remain valid
       * only for a write file.
       */
      if (global_capture_opts.has_autostop_duration) {
        cmdarg_err("A maximum capture time was specified, but "
          "a capture isn't being done.");
        exit_status = INVALID_OPTION;
        goto clean_exit;
      }
    } else {
      /*
       * "-r" wasn't specified, so we're doing a live capture.
       */
      if (perform_two_pass_analysis) {
        /* Two-pass analysis doesn't work with live capture since it requires us
         * to buffer packets until we've read all of them, but a live capture
         * has no useful/meaningful definition of "all" */
        cmdarg_err("Live captures do not support two-pass analysis.");
        exit_status = INVALID_OPTION;
        goto clean_exit;
      }

      if (global_capture_opts.saving_to_file) {
        /* They specified a "-w" flag, so we'll be saving to a capture file. */

        /* When capturing, we only support writing pcap or pcapng format. */
        if (out_file_type != WTAP_FILE_TYPE_SUBTYPE_PCAP &&
            out_file_type != WTAP_FILE_TYPE_SUBTYPE_PCAPNG) {
          cmdarg_err("Live captures can only be saved in pcap or pcapng format.");
          exit_status = INVALID_OPTION;
          goto clean_exit;
        }
        if (global_capture_opts.capture_comment &&
            out_file_type != WTAP_FILE_TYPE_SUBTYPE_PCAPNG) {
          cmdarg_err("A capture comment can only be written to a pcapng file.");
          exit_status = INVALID_OPTION;
          goto clean_exit;
        }
        if (global_capture_opts.multi_files_on) {
          /* Multiple-file mode doesn't work under certain conditions:
             a) it doesn't work if you're writing to the standard output;
             b) it doesn't work if you're writing to a pipe;
          */
          if (strcmp(global_capture_opts.save_file, "-") == 0) {
            cmdarg_err("Multiple capture files requested, but "
              "the capture is being written to the standard output.");
            exit_status = INVALID_OPTION;
            goto clean_exit;
          }
          if (global_capture_opts.output_to_pipe) {
            cmdarg_err("Multiple capture files requested, but "
              "the capture file is a pipe.");
            exit_status = INVALID_OPTION;
            goto clean_exit;
          }
          if (!global_capture_opts.has_autostop_filesize &&
              !global_capture_opts.has_file_duration &&
              !global_capture_opts.has_file_interval) {
            cmdarg_err("Multiple capture files requested, but "
              "no maximum capture file size, duration or interval was specified.");
            exit_status = INVALID_OPTION;
            goto clean_exit;
          }
        }
        /* Currently, we don't support read or display filters when capturing
           and saving the packets. */
        if (rfilter != NULL) {
          cmdarg_err("Read filters aren't supported when capturing and saving the captured packets.");
          exit_status = INVALID_OPTION;
          goto clean_exit;
        }
        if (dfilter != NULL) {
          cmdarg_err("Display filters aren't supported when capturing and saving the captured packets.");
          exit_status = INVALID_OPTION;
          goto clean_exit;
        }
        global_capture_opts.use_pcapng = (out_file_type == WTAP_FILE_TYPE_SUBTYPE_PCAPNG) ? TRUE : FALSE;
      } else {
        /* They didn't specify a "-w" flag, so we won't be saving to a
           capture file.  Check for options that only make sense if
           we're saving to a file. */
        if (global_capture_opts.has_autostop_filesize) {
          cmdarg_err("Maximum capture file size specified, but "
           "capture isn't being saved to a file.");
          exit_status = INVALID_OPTION;
          goto clean_exit;
        }
        if (global_capture_opts.multi_files_on) {
          cmdarg_err("Multiple capture files requested, but "
            "the capture isn't being saved to a file.");
          exit_status = INVALID_OPTION;
          goto clean_exit;
        }
        if (global_capture_opts.capture_comment) {
          cmdarg_err("A capture comment was specified, but "
            "the capture isn't being saved to a file.");
          exit_status = INVALID_OPTION;
          goto clean_exit;
        }
      }
    }
  }
#endif

  prefs_apply_all();
  start_exportobjects();

  {
    GSList* it = NULL;
    GSList *invalid_fields = output_fields_valid(output_fields);
    if (invalid_fields != NULL) {

      cmdarg_err("Some fields aren't valid:");
      for (it=invalid_fields; it != NULL; it = g_slist_next(it)) {
        cmdarg_err_cont("\t%s", (gchar *)it->data);
      }
      g_slist_free(invalid_fields);
      exit_status = INVALID_OPTION;
      goto clean_exit;
    }
  }

#ifdef HAVE_LIBPCAP
  /* We currently don't support taps, or printing dissected packets,
     if we're writing to a pipe. */
  if (global_capture_opts.saving_to_file &&
      global_capture_opts.output_to_pipe) {
    if (tap_listeners_require_dissection()) {
      cmdarg_err("Taps aren't supported when saving to a pipe.");
      exit_status = INVALID_OPTION;
      goto clean_exit;
    }
    if (print_packet_info) {
      cmdarg_err("Printing dissected packets isn't supported when saving to a pipe.");
      exit_status = INVALID_OPTION;
      goto clean_exit;
    }
  }
#endif

  if (ex_opt_count("read_format") > 0) {
    const gchar* name = ex_opt_get_next("read_format");
    in_file_type = open_info_name_to_type(name);
    if (in_file_type == WTAP_TYPE_AUTO) {
      cmdarg_err("\"%s\" isn't a valid read file format type", name? name : "");
      list_read_capture_types();
      exit_status = INVALID_OPTION;
      goto clean_exit;
    }
  }

  timestamp_set_type(global_dissect_options.time_format);

  /*
   * Enabled and disabled protocols and heuristic dissectors as per
   * command-line options.
   */
  if (!setup_enabled_and_disabled_protocols()) {
    exit_status = INVALID_OPTION;
    goto clean_exit;
  }

  /* Build the column format array */
  build_column_format_array(&cfile.cinfo, prefs_p->num_cols, TRUE);

#ifdef HAVE_LIBPCAP
  capture_opts_trim_snaplen(&global_capture_opts, MIN_PACKET_SIZE);
  capture_opts_trim_ring_num_files(&global_capture_opts);
#endif

  if (rfilter != NULL) {
    tshark_debug("Compiling read filter: '%s'", rfilter);
    if (!dfilter_compile(rfilter, &rfcode, &err_msg)) {
      cmdarg_err("%s", err_msg);
      g_free(err_msg);
      epan_cleanup();
      extcap_cleanup();
#ifdef HAVE_PCAP_OPEN_DEAD
      {
        pcap_t *pc;

        pc = pcap_open_dead(DLT_EN10MB, MIN_PACKET_SIZE);
        if (pc != NULL) {
          if (pcap_compile(pc, &fcode, rfilter, 0, 0) != -1) {
            cmdarg_err_cont(
              "  Note: That read filter code looks like a valid capture filter;\n"
              "        maybe you mixed them up?");
          }
          pcap_close(pc);
        }
      }
#endif
      exit_status = INVALID_INTERFACE;
      goto clean_exit;
    }
  }
  cfile.rfcode = rfcode;

  if (dfilter != NULL) {
    tshark_debug("Compiling display filter: '%s'", dfilter);
    if (!dfilter_compile(dfilter, &dfcode, &err_msg)) {
      cmdarg_err("%s", err_msg);
      g_free(err_msg);
      epan_cleanup();
      extcap_cleanup();
#ifdef HAVE_PCAP_OPEN_DEAD
      {
        pcap_t *pc;

        pc = pcap_open_dead(DLT_EN10MB, MIN_PACKET_SIZE);
        if (pc != NULL) {
          if (pcap_compile(pc, &fcode, dfilter, 0, 0) != -1) {
            cmdarg_err_cont(
              "  Note: That display filter code looks like a valid capture filter;\n"
              "        maybe you mixed them up?");
          }
          pcap_close(pc);
        }
      }
#endif
      exit_status = INVALID_FILTER;
      goto clean_exit;
    }
  }
  cfile.dfcode = dfcode;

  if (print_packet_info) {
    /* If we're printing as text or PostScript, we have
       to create a print stream. */
    if (output_action == WRITE_TEXT) {
      switch (print_format) {

      case PR_FMT_TEXT:
        print_stream = print_stream_text_stdio_new(stdout);
        break;

      case PR_FMT_PS:
        print_stream = print_stream_ps_stdio_new(stdout);
        break;

      default:
        g_assert_not_reached();
      }
    }
  }

  /* PDU export requested. Take the ownership of the '-w' file, apply tap
  * filters and start tapping. */
  if (pdu_export_arg) {
      const char *exp_pdu_tap_name = pdu_export_arg;
      const char *exp_pdu_filter = dfilter; /* may be NULL to disable filter */
      char       *exp_pdu_error;
      int         exp_fd;
      char       *comment;

      if (!cf_name) {
          cmdarg_err("PDUs export requires a capture file (specify with -r).");
          exit_status = INVALID_OPTION;
          goto clean_exit;
      }
      /* Take ownership of the '-w' output file. */
#ifdef HAVE_LIBPCAP
      exp_pdu_filename = global_capture_opts.save_file;
      global_capture_opts.save_file = NULL;
#else
      exp_pdu_filename = output_file_name;
      output_file_name = NULL;
#endif
      if (exp_pdu_filename == NULL) {
          cmdarg_err("PDUs export requires an output file (-w).");
          exit_status = INVALID_OPTION;
          goto clean_exit;
      }

      exp_pdu_error = exp_pdu_pre_open(exp_pdu_tap_name, exp_pdu_filter,
          &exp_pdu_tap_data);
      if (exp_pdu_error) {
          cmdarg_err("Cannot register tap: %s", exp_pdu_error);
          g_free(exp_pdu_error);
          exit_status = INVALID_TAP;
          goto clean_exit;
      }

      exp_fd = ws_open(exp_pdu_filename, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644);
      if (exp_fd == -1) {
          cmdarg_err("%s: %s", exp_pdu_filename, file_open_error_message(errno, TRUE));
          exit_status = INVALID_FILE;
          goto clean_exit;
      }

      /* Activate the export PDU tap */
      comment = g_strdup_printf("Dump of PDUs from %s", cf_name);
      err = exp_pdu_open(&exp_pdu_tap_data, exp_fd, comment);
      if (err != 0) {
          cfile_dump_open_failure_message("TShark", exp_pdu_filename, err,
                                          WTAP_FILE_TYPE_SUBTYPE_PCAPNG);
          g_free(comment);
          exit_status = INVALID_EXPORT;
          goto clean_exit;
      }
  }

  tshark_debug("tshark: do_dissection = %s", do_dissection ? "TRUE" : "FALSE");

  if (cf_name) {
    tshark_debug("tshark: Opening capture file: %s", cf_name);
    /*
     * We're reading a capture file.
     */
    if (cf_open(&cfile, cf_name, in_file_type, FALSE, &err) != CF_OK) {
      epan_cleanup();
      extcap_cleanup();
      exit_status = INVALID_FILE;
      goto clean_exit;
    }

    /* Start statistics taps; we do so after successfully opening the
       capture file, so we know we have something to compute stats
       on, and after registering all dissectors, so that MATE will
       have registered its field array so we can have a tap filter
       with one of MATE's late-registered fields as part of the
       filter. */
    start_requested_stats();

    /* Do we need to do dissection of packets?  That depends on, among
       other things, what taps are listening, so determine that after
       starting the statistics taps. */
    do_dissection = must_do_dissection(rfcode, dfcode, pdu_export_arg);

    /* Process the packets in the file */
    tshark_debug("tshark: invoking process_cap_file() to process the packets");
    TRY {
#ifdef HAVE_LIBPCAP
      success = process_cap_file(&cfile, global_capture_opts.save_file, out_file_type, out_file_name_res,
          global_capture_opts.has_autostop_packets ? global_capture_opts.autostop_packets : 0,
          global_capture_opts.has_autostop_filesize ? global_capture_opts.autostop_filesize : 0);
#else
      success = process_cap_file(&cfile, output_file_name, out_file_type, out_file_name_res, 0, 0);
#endif
    }
    CATCH(OutOfMemoryError) {
      fprintf(stderr,
              "Out Of Memory.\n"
              "\n"
              "Sorry, but TShark has to terminate now.\n"
              "\n"
              "More information and workarounds can be found at\n"
              "https://wiki.wireshark.org/KnownBugs/OutOfMemory\n");
      success = FALSE;
    }
    ENDTRY;

    if (!success) {
      /* We still dump out the results of taps, etc., as we might have
         read some packets; however, we exit with an error status. */
      exit_status = 2;
    }

    if (pdu_export_arg) {
        err = exp_pdu_close(&exp_pdu_tap_data);
        if (err) {
            cfile_close_failure_message(exp_pdu_filename, err);
            exit_status = 2;
        }
        g_free(pdu_export_arg);
    }
  } else {
    tshark_debug("tshark: no capture file specified");
    /* No capture file specified, so we're supposed to do a live capture
       or get a list of link-layer types for a live capture device;
       do we have support for live captures? */

    /* if no interface was specified, pick a default */
    exit_status = capture_opts_default_iface_if_necessary(&global_capture_opts,
        ((prefs_p->capture_device) && (*prefs_p->capture_device != '\0'))
        ? get_if_name(prefs_p->capture_device) : NULL);
    if (exit_status != 0) {
      goto clean_exit;
    }

    /* if requested, list the link layer types and exit */
    if (caps_queries) {
        guint i;

        /* Get the list of link-layer types for the capture devices. */
        for (i = 0; i < global_capture_opts.ifaces->len; i++) {
          interface_options *interface_opts;
          if_capabilities_t *caps;
          char *auth_str = NULL;
          int if_caps_queries = caps_queries;

          interface_opts = &g_array_index(global_capture_opts.ifaces, interface_options, i);
          caps = capture_get_if_capabilities(interface_opts->name,
              interface_opts->monitor_mode, auth_str, &err_str, NULL);
          g_free(auth_str);
          if (caps == NULL) {
            cmdarg_err("%s", err_str);
            g_free(err_str);
            exit_status = INVALID_CAPABILITY;
            goto clean_exit;
          }
          if ((if_caps_queries & CAPS_QUERY_LINK_TYPES) && caps->data_link_types == NULL) {
            cmdarg_err("The capture device \"%s\" has no data link types.", interface_opts->name);
            exit_status = INVALID_DATA_LINK;
            goto clean_exit;
          }
          if ((if_caps_queries & CAPS_QUERY_TIMESTAMP_TYPES) && caps->timestamp_types == NULL) {
            cmdarg_err("The capture device \"%s\" has no timestamp types.", interface_opts->name);
            exit_status = INVALID_TIMESTAMP_TYPE;
            goto clean_exit;
          }
          if (interface_opts->monitor_mode)
                if_caps_queries |= CAPS_MONITOR_MODE;
          capture_opts_print_if_capabilities(caps, interface_opts->name, if_caps_queries);
          free_if_capabilities(caps);
        }
        exit_status = EXIT_SUCCESS;
        goto clean_exit;
    }

    if (!ws_isatty(ws_fileno(stderr)))
      print_packet_counts = FALSE;
    else if (print_packet_info && ws_isatty(ws_fileno(stdout)))
      print_packet_counts = FALSE;
    else if (quiet)
      print_packet_counts = FALSE;
    else
      print_packet_counts = TRUE;

    if (print_packet_info) {
      if (!write_preamble(&cfile)) {
        show_print_file_io_error(errno);
        exit_status = INVALID_FILE;
        goto clean_exit;
      }
    }

    tshark_debug("tshark: performing live capture");
    start_requested_stats();
    do_dissection = must_do_dissection(rfcode, dfcode, pdu_export_arg);
    capture();
    exit_status = global_capture_session.fork_child_status;

    if (print_packet_info) {
      if (!write_finale()) {
        show_print_file_io_error(errno);
      }
    }
  }

  g_free(cf_name);

  if (cfile.provider.frames != NULL) {
    free_frame_data_sequence(cfile.provider.frames);
    cfile.provider.frames = NULL;
  }

  draw_tap_listeners(TRUE);
  funnel_dump_all_text_windows();
  epan_free(cfile.epan);
  epan_cleanup();
  extcap_cleanup();

  output_fields_free(output_fields);
  output_fields = NULL;

clean_exit:
  destroy_print_stream(print_stream);
#ifdef HAVE_LIBPCAP
  capture_opts_cleanup(&global_capture_opts);
#endif
  col_cleanup(&cfile.cinfo);
  free_filter_lists();
  wtap_cleanup();
  free_progdirs();
  cf_close(&cfile);
  return exit_status;
}

/*#define USE_BROKEN_G_MAIN_LOOP*/

#ifdef USE_BROKEN_G_MAIN_LOOP
  GMainLoop *loop;
#else
  gboolean loop_running = FALSE;
#endif
  guint32 packet_count = 0;


typedef struct pipe_input_tag {
  gint             source;
  gpointer         user_data;
  ws_process_id   *child_process;
  pipe_input_cb_t  input_cb;
  guint            pipe_input_id;
#ifdef _WIN32
  GMutex          *callback_running;
#endif
} pipe_input_t;

static pipe_input_t pipe_input;

#ifdef _WIN32
/* The timer has expired, see if there's stuff to read from the pipe,
   if so, do the callback */
static gint
pipe_timer_cb(gpointer data)
{
  HANDLE        handle;
  DWORD         avail        = 0;
  gboolean      result;
  DWORD         childstatus;
  pipe_input_t *pipe_input_p = data;
  gint          iterations   = 0;

  g_mutex_lock (pipe_input_p->callback_running);

  /* try to read data from the pipe only 5 times, to avoid blocking */
  while(iterations < 5) {
    /*g_log(NULL, G_LOG_LEVEL_DEBUG, "pipe_timer_cb: new iteration");*/

    /* Oddly enough although Named pipes don't work on win9x,
       PeekNamedPipe does !!! */
    handle = (HANDLE) _get_osfhandle (pipe_input_p->source);
    result = PeekNamedPipe(handle, NULL, 0, NULL, &avail, NULL);

    /* Get the child process exit status */
    GetExitCodeProcess((HANDLE)*(pipe_input_p->child_process),
                       &childstatus);

    /* If the Peek returned an error, or there are bytes to be read
       or the childwatcher thread has terminated then call the normal
       callback */
    if (!result || avail > 0 || childstatus != STILL_ACTIVE) {

      /*g_log(NULL, G_LOG_LEVEL_DEBUG, "pipe_timer_cb: data avail");*/

      /* And call the real handler */
      if (!pipe_input_p->input_cb(pipe_input_p->source, pipe_input_p->user_data)) {
        g_log(NULL, G_LOG_LEVEL_DEBUG, "pipe_timer_cb: input pipe closed, iterations: %u", iterations);
        /* pipe closed, return false so that the timer is stopped */
        g_mutex_unlock (pipe_input_p->callback_running);
        return FALSE;
      }
    }
    else {
      /*g_log(NULL, G_LOG_LEVEL_DEBUG, "pipe_timer_cb: no data avail");*/
      /* No data, stop now */
      break;
    }

    iterations++;
  }

  /*g_log(NULL, G_LOG_LEVEL_DEBUG, "pipe_timer_cb: finished with iterations: %u, new timer", iterations);*/

  g_mutex_unlock (pipe_input_p->callback_running);

  /* we didn't stopped the timer, so let it run */
  return TRUE;
}
#endif


void
pipe_input_set_handler(gint source, gpointer user_data, ws_process_id *child_process, pipe_input_cb_t input_cb)
{

  pipe_input.source         = source;
  pipe_input.child_process  = child_process;
  pipe_input.user_data      = user_data;
  pipe_input.input_cb       = input_cb;

#ifdef _WIN32
#if GLIB_CHECK_VERSION(2,31,0)
  pipe_input.callback_running = g_malloc(sizeof(GMutex));
  g_mutex_init(pipe_input.callback_running);
#else
  pipe_input.callback_running = g_mutex_new();
#endif
  /* Tricky to use pipes in win9x, as no concept of wait.  NT can
     do this but that doesn't cover all win32 platforms.  GTK can do
     this but doesn't seem to work over processes.  Attempt to do
     something similar here, start a timer and check for data on every
     timeout. */
  /*g_log(NULL, G_LOG_LEVEL_DEBUG, "pipe_input_set_handler: new");*/
  pipe_input.pipe_input_id = g_timeout_add(200, pipe_timer_cb, &pipe_input);
#endif
}

static const nstime_t *
tshark_get_frame_ts(struct packet_provider_data *prov, guint32 frame_num)
{
  if (prov->ref && prov->ref->num == frame_num)
    return &prov->ref->abs_ts;

  if (prov->prev_dis && prov->prev_dis->num == frame_num)
    return &prov->prev_dis->abs_ts;

  if (prov->prev_cap && prov->prev_cap->num == frame_num)
    return &prov->prev_cap->abs_ts;

  if (prov->frames) {
     frame_data *fd = frame_data_sequence_find(prov->frames, frame_num);

     return (fd) ? &fd->abs_ts : NULL;
  }

  return NULL;
}

static epan_t *
tshark_epan_new(capture_file *cf)
{
  static const struct packet_provider_funcs funcs = {
    tshark_get_frame_ts,
    cap_file_provider_get_interface_name,
    cap_file_provider_get_interface_description,
    NULL,
  };

  return epan_new(&cf->provider, &funcs);
}

#ifdef HAVE_LIBPCAP
static gboolean
capture(void)
{
  gboolean          ret;
  guint             i;
  GString          *str;
  struct sigaction  action, oldaction;

  /* Create new dissection section. */
  epan_free(cfile.epan);
  cfile.epan = tshark_epan_new(&cfile);

  memset(&action, 0, sizeof(action));
  action.sa_handler = capture_cleanup;
  action.sa_flags = SA_RESTART;
  sigemptyset(&action.sa_mask);
  sigaction(SIGTERM, &action, NULL);
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGHUP, NULL, &oldaction);
  if (oldaction.sa_handler == SIG_DFL)
    sigaction(SIGHUP, &action, NULL);

  global_capture_session.state = CAPTURE_PREPARING;

  /* Let the user know which interfaces were chosen. */
  for (i = 0; i < global_capture_opts.ifaces->len; i++) {
    interface_options *interface_opts;

    interface_opts = &g_array_index(global_capture_opts.ifaces, interface_options, i);
    interface_opts->descr = get_interface_descriptive_name(interface_opts->name);
  }
  str = get_iface_list_string(&global_capture_opts, IFLIST_QUOTE_IF_DESCRIPTION);
  if (really_quiet == FALSE)
    fprintf(stderr, "Capturing on %s\n", str->str);
  fflush(stderr);
  g_string_free(str, TRUE);

  ret = sync_pipe_start(&global_capture_opts, &global_capture_session, &global_info_data, NULL);

  if (!ret)
    return FALSE;

  /* the actual capture loop
   *
   * XXX - glib doesn't seem to provide any event based loop handling.
   *
   * XXX - for whatever reason,
   * calling g_main_loop_new() ends up in 100% cpu load.
   *
   * But that doesn't matter: in UNIX we can use select() to find an input
   * source with something to do.
   *
   * But that doesn't matter because we're in a CLI (that doesn't need to
   * update a GUI or something at the same time) so it's OK if we block
   * trying to read from the pipe.
   *
   * So all the stuff in USE_TSHARK_SELECT could be removed unless I'm
   * wrong (but I leave it there in case I am...).
   */

#ifdef USE_TSHARK_SELECT
  FD_ZERO(&readfds);
  FD_SET(pipe_input.source, &readfds);
#endif

  loop_running = TRUE;

  TRY
  {
    while (loop_running)
    {
#ifdef USE_TSHARK_SELECT
      ret = select(pipe_input.source+1, &readfds, NULL, NULL, NULL);

      if (ret == -1)
      {
        fprintf(stderr, "%s: %s\n", "select()", g_strerror(errno));
        return TRUE;
      } else if (ret == 1) {
#endif
        printf("input callback(%d,%p)\n", pipe_input.source, pipe_input.user_data);
        /* Call the real handler */
        if (!pipe_input.input_cb(pipe_input.source, pipe_input.user_data)) {
          g_log(NULL, G_LOG_LEVEL_DEBUG, "input pipe closed");
          return FALSE;
        }
#ifdef USE_TSHARK_SELECT
      }
#endif
    }
  }
  CATCH(OutOfMemoryError) {
    fprintf(stderr,
            "Out Of Memory.\n"
            "\n"
            "Sorry, but TShark has to terminate now.\n"
            "\n"
            "More information and workarounds can be found at\n"
            "https://wiki.wireshark.org/KnownBugs/OutOfMemory\n");
    exit(1);
  }
  ENDTRY;
  return TRUE;
}

/* capture child detected an error */
void
capture_input_error_message(capture_session *cap_session _U_, char *error_msg, char *secondary_error_msg)
{
  cmdarg_err("%s", error_msg);
  cmdarg_err_cont("%s", secondary_error_msg);
}


/* capture child detected an capture filter related error */
void
capture_input_cfilter_error_message(capture_session *cap_session, guint i, char *error_message)
{
  capture_options *capture_opts = cap_session->capture_opts;
  dfilter_t         *rfcode = NULL;
  interface_options *interface_opts;

  g_assert(i < capture_opts->ifaces->len);
  interface_opts = &g_array_index(capture_opts->ifaces, interface_options, i);

  if (dfilter_compile(interface_opts->cfilter, &rfcode, NULL) && rfcode != NULL) {
    cmdarg_err(
      "Invalid capture filter \"%s\" for interface '%s'.\n"
      "\n"
      "That string looks like a valid display filter; however, it isn't a valid\n"
      "capture filter (%s).\n"
      "\n"
      "Note that display filters and capture filters don't have the same syntax,\n"
      "so you can't use most display filter expressions as capture filters.\n"
      "\n"
      "See the User's Guide for a description of the capture filter syntax.",
      interface_opts->cfilter, interface_opts->descr, error_message);
    dfilter_free(rfcode);
  } else {
    cmdarg_err(
      "Invalid capture filter \"%s\" for interface '%s'.\n"
      "\n"
      "That string isn't a valid capture filter (%s).\n"
      "See the User's Guide for a description of the capture filter syntax.",
      interface_opts->cfilter, interface_opts->descr, error_message);
  }
}


/* capture child tells us we have a new (or the first) capture file */
gboolean
capture_input_new_file(capture_session *cap_session, gchar *new_file)
{
  capture_options *capture_opts = cap_session->capture_opts;
  capture_file *cf = (capture_file *) cap_session->cf;
  gboolean is_tempfile;
  int      err;

  if (cap_session->state == CAPTURE_PREPARING) {
    g_log(LOG_DOMAIN_CAPTURE, G_LOG_LEVEL_MESSAGE, "Capture started.");
  }
  g_log(LOG_DOMAIN_CAPTURE, G_LOG_LEVEL_MESSAGE, "File: \"%s\"", new_file);

  g_assert(cap_session->state == CAPTURE_PREPARING || cap_session->state == CAPTURE_RUNNING);

  /* free the old filename */
  if (capture_opts->save_file != NULL) {

    /* we start a new capture file, close the old one (if we had one before) */
    if (cf->state != FILE_CLOSED) {
      if (cf->provider.wth != NULL) {
        wtap_close(cf->provider.wth);
        cf->provider.wth = NULL;
      }
      cf->state = FILE_CLOSED;
    }

    g_free(capture_opts->save_file);
    is_tempfile = FALSE;

    epan_free(cf->epan);
    cf->epan = tshark_epan_new(cf);
  } else {
    /* we didn't had a save_file before, must be a tempfile */
    is_tempfile = TRUE;
  }

  /* save the new filename */
  capture_opts->save_file = g_strdup(new_file);

  /* if we are in real-time mode, open the new file now */
  if (do_dissection) {
    /* this is probably unecessary, but better safe than sorry */
    ((capture_file *)cap_session->cf)->open_type = WTAP_TYPE_AUTO;
    /* Attempt to open the capture file and set up to read from it. */
    switch(cf_open((capture_file *)cap_session->cf, capture_opts->save_file, WTAP_TYPE_AUTO, is_tempfile, &err)) {
    case CF_OK:
      break;
    case CF_ERROR:
      /* Don't unlink (delete) the save file - leave it around,
         for debugging purposes. */
      g_free(capture_opts->save_file);
      capture_opts->save_file = NULL;
      return FALSE;
    }
  }

  cap_session->state = CAPTURE_RUNNING;

  return TRUE;
}


/* capture child tells us we have new packets to read */
void
capture_input_new_packets(capture_session *cap_session, int to_read)
{
  gboolean      ret;
  int           err;
  gchar        *err_info;
  gint64        data_offset;
  capture_file *cf = (capture_file *)cap_session->cf;
  gboolean      filtering_tap_listeners;
  guint         tap_flags;

#ifdef SIGINFO
  /*
   * Prevent a SIGINFO handler from writing to the standard error while
   * we're doing so or writing to the standard output; instead, have it
   * just set a flag telling us to print that information when we're done.
   */
  infodelay = TRUE;
#endif /* SIGINFO */

  /* Do we have any tap listeners with filters? */
  filtering_tap_listeners = have_filtering_tap_listeners();

  /* Get the union of the flags for all tap listeners. */
  tap_flags = union_of_tap_listener_flags();

  if (do_dissection) {
    gboolean create_proto_tree;
    epan_dissect_t *edt;

    /*
     * Determine whether we need to create a protocol tree.
     * We do if:
     *
     *    we're going to apply a read filter;
     *
     *    we're going to apply a display filter;
     *
     *    we're going to print the protocol tree;
     *
     *    one of the tap listeners is going to apply a filter;
     *
     *    one of the tap listeners requires a protocol tree;
     *
     *    a postdissector wants field values or protocols
     *    on the first pass;
     *
     *    we have custom columns (which require field values, which
     *    currently requires that we build a protocol tree).
     */
    create_proto_tree =
      (cf->rfcode || cf->dfcode || print_details || filtering_tap_listeners ||
        (tap_flags & TL_REQUIRES_PROTO_TREE) || postdissectors_want_hfids() ||
        have_custom_cols(&cf->cinfo) || dissect_color);

    /* The protocol tree will be "visible", i.e., printed, only if we're
       printing packet details, which is true if we're printing stuff
       ("print_packet_info" is true) and we're in verbose mode
       ("packet_details" is true). */
    edt = epan_dissect_new(cf->epan, create_proto_tree, print_packet_info && print_details);

    while (to_read-- && cf->provider.wth) {
      wtap_cleareof(cf->provider.wth);
      ret = wtap_read(cf->provider.wth, &err, &err_info, &data_offset);
      reset_epan_mem(cf, edt, create_proto_tree, print_packet_info && print_details);
      if (ret == FALSE) {
        /* read from file failed, tell the capture child to stop */
        sync_pipe_stop(cap_session);
        wtap_close(cf->provider.wth);
        cf->provider.wth = NULL;
      } else {
        ret = process_packet_single_pass(cf, edt, data_offset,
                                         wtap_get_rec(cf->provider.wth),
                                         wtap_get_buf_ptr(cf->provider.wth), tap_flags);
      }
      if (ret != FALSE) {
        /* packet successfully read and gone through the "Read Filter" */
        packet_count++;
      }
    }

    epan_dissect_free(edt);

  } else {
    /*
     * Dumpcap's doing all the work; we're not doing any dissection.
     * Count all the packets it wrote.
     */
    packet_count += to_read;
  }

  if (print_packet_counts) {
      /* We're printing packet counts. */
      if (packet_count != 0) {
        fprintf(stderr, "\r%u ", packet_count);
        /* stderr could be line buffered */
        fflush(stderr);
      }
  }

#ifdef SIGINFO
  /*
   * Allow SIGINFO handlers to write.
   */
  infodelay = FALSE;

  /*
   * If a SIGINFO handler asked us to write out capture counts, do so.
   */
  if (infoprint)
    report_counts();
#endif /* SIGINFO */
}

static void
report_counts(void)
{
  if ((print_packet_counts == FALSE) && (really_quiet == FALSE)) {
    /* Report the count only if we aren't printing a packet count
       as packets arrive. */
      fprintf(stderr, "%u packet%s captured\n", packet_count,
            plurality(packet_count, "", "s"));
  }
#ifdef SIGINFO
  infoprint = FALSE; /* we just reported it */
#endif /* SIGINFO */
}

#ifdef SIGINFO
static void
report_counts_siginfo(int signum _U_)
{
  int sav_errno = errno;
  /* If we've been told to delay printing, just set a flag asking
     that we print counts (if we're supposed to), otherwise print
     the count of packets captured (if we're supposed to). */
  if (infodelay)
    infoprint = TRUE;
  else
    report_counts();
  errno = sav_errno;
}
#endif /* SIGINFO */


/* capture child detected any packet drops? */
void
capture_input_drops(capture_session *cap_session _U_, guint32 dropped)
{
  if (print_packet_counts) {
    /* We're printing packet counts to stderr.
       Send a newline so that we move to the line after the packet count. */
    fprintf(stderr, "\n");
  }

  if (dropped != 0) {
    /* We're printing packet counts to stderr.
       Send a newline so that we move to the line after the packet count. */
    fprintf(stderr, "%u packet%s dropped\n", dropped, plurality(dropped, "", "s"));
  }
}


/*
 * Capture child closed its side of the pipe, report any error and
 * do the required cleanup.
 */
void
capture_input_closed(capture_session *cap_session, gchar *msg)
{
  capture_file *cf = (capture_file *) cap_session->cf;

  if (msg != NULL)
    fprintf(stderr, "tshark: %s\n", msg);

  report_counts();

  if (cf != NULL && cf->provider.wth != NULL) {
    wtap_close(cf->provider.wth);
    if (cf->is_tempfile) {
      ws_unlink(cf->filename);
    }
  }
#ifdef USE_BROKEN_G_MAIN_LOOP
  g_main_loop_quit(loop);
#else
  loop_running = FALSE;
#endif
}

static void
capture_cleanup(int signum _U_)
{
  sync_pipe_stop(&global_capture_session);
}

#endif /* HAVE_LIBPCAP */

static gboolean
process_cap_file(capture_file *cf, char *save_file, int out_file_type,
    gboolean out_file_name_res, int max_packet_count, gint64 max_byte_count)
{
  gboolean     success = TRUE;
  gint         linktype;
  int          snapshot_length;
  wtap_dumper *pdh;
  guint32      framenum;
  int          err = 0, err_pass1 = 0;
  gchar       *err_info = NULL, *err_info_pass1 = NULL;
  gint64       data_offset;
  gboolean     filtering_tap_listeners;
  guint        tap_flags;
  GArray                      *shb_hdrs = NULL;
  wtapng_iface_descriptions_t *idb_inf = NULL;
  GArray                      *nrb_hdrs = NULL;
  wtap_rec     rec;
  Buffer       buf;
  epan_dissect_t *edt = NULL;
  char                        *shb_user_appl;

  wtap_rec_init(&rec);

  idb_inf = wtap_file_get_idb_info(cf->provider.wth);
#ifdef PCAP_NG_DEFAULT
  if (idb_inf->interface_data->len > 1) {
    linktype = WTAP_ENCAP_PER_PACKET;
  } else {
    linktype = wtap_file_encap(cf->provider.wth);
  }
#else
  linktype = wtap_file_encap(cf->provider.wth);
#endif
  if (save_file != NULL) {
    /* Set up to write to the capture file. */
    snapshot_length = wtap_snapshot_length(cf->provider.wth);
    if (snapshot_length == 0) {
      /* Snapshot length of input file not known. */
      snapshot_length = WTAP_MAX_PACKET_SIZE_STANDARD;
    }
    tshark_debug("tshark: snapshot_length = %d", snapshot_length);

    shb_hdrs = wtap_file_get_shb_for_new_file(cf->provider.wth);
    nrb_hdrs = wtap_file_get_nrb_for_new_file(cf->provider.wth);

    /* If we don't have an application name add Tshark */
    if (wtap_block_get_string_option_value(g_array_index(shb_hdrs, wtap_block_t, 0), OPT_SHB_USERAPPL, &shb_user_appl) != WTAP_OPTTYPE_SUCCESS) {
        /* this is free'd by wtap_block_free() later */
        wtap_block_add_string_option_format(g_array_index(shb_hdrs, wtap_block_t, 0), OPT_SHB_USERAPPL, "TShark (Wireshark) %s", get_ws_vcs_version_info());
    }

    if (linktype != WTAP_ENCAP_PER_PACKET &&
        out_file_type == WTAP_FILE_TYPE_SUBTYPE_PCAP) {
        tshark_debug("tshark: writing PCAP format to %s", save_file);
        if (strcmp(save_file, "-") == 0) {
          /* Write to the standard output. */
          pdh = wtap_dump_open_stdout(out_file_type, linktype,
              snapshot_length, FALSE /* compressed */, &err);
        } else {
          pdh = wtap_dump_open(save_file, out_file_type, linktype,
              snapshot_length, FALSE /* compressed */, &err);
        }
    }
    else {
        tshark_debug("tshark: writing format type %d, to %s", out_file_type, save_file);
        if (strcmp(save_file, "-") == 0) {
          /* Write to the standard output. */
          pdh = wtap_dump_open_stdout_ng(out_file_type, linktype,
              snapshot_length, FALSE /* compressed */, shb_hdrs, idb_inf, nrb_hdrs, &err);
        } else {
          pdh = wtap_dump_open_ng(save_file, out_file_type, linktype,
              snapshot_length, FALSE /* compressed */, shb_hdrs, idb_inf, nrb_hdrs, &err);
        }
    }

    g_free(idb_inf);
    idb_inf = NULL;

    if (pdh == NULL) {
      /* We couldn't set up to write to the capture file. */
      cfile_dump_open_failure_message("TShark", save_file, err, out_file_type);
      success = FALSE;
      goto out;
    }
  } else {
    /* Set up to print packet information. */
    if (print_packet_info) {
      if (!write_preamble(cf)) {
        show_print_file_io_error(errno);
        success = FALSE;
        goto out;
      }
    }
    g_free(idb_inf);
    idb_inf = NULL;
    pdh = NULL;
  }

  /* Do we have any tap listeners with filters? */
  filtering_tap_listeners = have_filtering_tap_listeners();

  /* Get the union of the flags for all tap listeners. */
  tap_flags = union_of_tap_listener_flags();

  if (perform_two_pass_analysis) {
    frame_data *fdata;

    tshark_debug("tshark: perform_two_pass_analysis, do_dissection=%s", do_dissection ? "TRUE" : "FALSE");

    /* Allocate a frame_data_sequence for all the frames. */
    cf->provider.frames = new_frame_data_sequence();

    if (do_dissection) {
      gboolean create_proto_tree;

      /*
       * Determine whether we need to create a protocol tree.
       * We do if:
       *
       *    we're going to apply a read filter;
       *
       *    we're going to apply a display filter;
       *
       *    a postdissector wants field values or protocols
       *    on the first pass.
       */
      create_proto_tree =
        (cf->rfcode != NULL || cf->dfcode != NULL || postdissectors_want_hfids() || dissect_color);

      tshark_debug("tshark: create_proto_tree = %s", create_proto_tree ? "TRUE" : "FALSE");

      /* We're not going to display the protocol tree on this pass,
         so it's not going to be "visible". */
      edt = epan_dissect_new(cf->epan, create_proto_tree, FALSE);
    }

    tshark_debug("tshark: reading records for first pass");
    while (wtap_read(cf->provider.wth, &err, &err_info, &data_offset)) {
    }

    /*
     * If we got a read error on the first pass, remember the error, so
     * but do the second pass, so we can at least process the packets we
     * read, and then report the first-pass error after the second pass
     * (and before we report any second-pass errors), so all the the
     * errors show up at the end.
     */
    if (err != 0) {
      err_pass1 = err;
      err_info_pass1 = err_info;
      err = 0;
      err_info = NULL;
    }

    if (edt) {
      epan_dissect_free(edt);
      edt = NULL;
    }

    /* Close the sequential I/O side, to free up memory it requires. */
    wtap_sequential_close(cf->provider.wth);

    /* Allow the protocol dissectors to free up memory that they
     * don't need after the sequential run-through of the packets. */
    postseq_cleanup_all_protocols();

    cf->provider.prev_dis = NULL;
    cf->provider.prev_cap = NULL;
    ws_buffer_init(&buf, 1500);

    tshark_debug("tshark: done with first pass");

    if (do_dissection) {
      gboolean create_proto_tree;

      /*
       * Determine whether we need to create a protocol tree.
       * We do if:
       *
       *    we're going to apply a display filter;
       *
       *    we're going to print the protocol tree;
       *
       *    one of the tap listeners requires a protocol tree;
       *
       *    we have custom columns (which require field values, which
       *    currently requires that we build a protocol tree).
       */
      create_proto_tree =
        (cf->dfcode || print_details || filtering_tap_listeners ||
         (tap_flags & TL_REQUIRES_PROTO_TREE) || have_custom_cols(&cf->cinfo) || dissect_color);

      tshark_debug("tshark: create_proto_tree = %s", create_proto_tree ? "TRUE" : "FALSE");

      /* The protocol tree will be "visible", i.e., printed, only if we're
         printing packet details, which is true if we're printing stuff
         ("print_packet_info" is true) and we're in verbose mode
         ("packet_details" is true). */
      edt = epan_dissect_new(cf->epan, create_proto_tree, print_packet_info && print_details);
    }

    for (framenum = 1; err == 0 && framenum <= cf->count; framenum++) {
      fdata = frame_data_sequence_find(cf->provider.frames, framenum);
      if (wtap_seek_read(cf->provider.wth, fdata->file_off, &rec, &buf, &err,
                         &err_info)) {
        tshark_debug("tshark: invoking process_packet_second_pass() for frame #%d", framenum);
      }
    }

    if (edt) {
      epan_dissect_free(edt);
      edt = NULL;
    }

    ws_buffer_free(&buf);

    tshark_debug("tshark: done with second pass");
  }
  else {
    /* !perform_two_pass_analysis */
    framenum = 0;
    gboolean create_proto_tree = FALSE;
    tshark_debug("tshark: perform one pass analysis, do_dissection=%s", do_dissection ? "TRUE" : "FALSE");

    if (do_dissection) {
      /*
       * Determine whether we need to create a protocol tree.
       * We do if:
       *
       *    we're going to apply a read filter;
       *
       *    we're going to apply a display filter;
       *
       *    we're going to print the protocol tree;
       *
       *    one of the tap listeners is going to apply a filter;
       *
       *    one of the tap listeners requires a protocol tree;
       *
       *    a postdissector wants field values or protocols
       *    on the first pass;
       *
       *    we have custom columns (which require field values, which
       *    currently requires that we build a protocol tree).
       */
      create_proto_tree =
        (cf->rfcode || cf->dfcode || print_details || filtering_tap_listeners ||
          (tap_flags & TL_REQUIRES_PROTO_TREE) || postdissectors_want_hfids() ||
          have_custom_cols(&cf->cinfo) || dissect_color);

      tshark_debug("tshark: create_proto_tree = %s", create_proto_tree ? "TRUE" : "FALSE");

      /* The protocol tree will be "visible", i.e., printed, only if we're
         printing packet details, which is true if we're printing stuff
         ("print_packet_info" is true) and we're in verbose mode
         ("packet_details" is true). */
      edt = epan_dissect_new(cf->epan, create_proto_tree, print_packet_info && print_details);
    }

    while (wtap_read(cf->provider.wth, &err, &err_info, &data_offset)) {
      framenum++;

      tshark_debug("tshark: processing packet #%d", framenum);

      reset_epan_mem(cf, edt, create_proto_tree, print_packet_info && print_details);

      if (process_packet_single_pass(cf, edt, data_offset, wtap_get_rec(cf->provider.wth),
                                     wtap_get_buf_ptr(cf->provider.wth), tap_flags)) {
        /* Either there's no read filtering or this packet passed the
           filter, so, if we're writing to a capture file, write
           this packet out. */
        if (pdh != NULL) {
          tshark_debug("tshark: writing packet #%d to outfile", framenum);
          if (!wtap_dump(pdh, wtap_get_rec(cf->provider.wth), wtap_get_buf_ptr(cf->provider.wth), &err, &err_info)) {
            /* Error writing to a capture file */
            tshark_debug("tshark: error writing to a capture file (%d)", err);
            cfile_write_failure_message("TShark", cf->filename, save_file,
                                        err, err_info, framenum, out_file_type);
            wtap_dump_close(pdh, &err);
            wtap_block_array_free(shb_hdrs);
            wtap_block_array_free(nrb_hdrs);
            exit(2);
          }
        }
      }
      /* Stop reading if we have the maximum number of packets;
       * When the -c option has not been used, max_packet_count
       * starts at 0, which practically means, never stop reading.
       * (unless we roll over max_packet_count ?)
       */
      if ( (--max_packet_count == 0) || (max_byte_count != 0 && data_offset >= max_byte_count)) {
        tshark_debug("tshark: max_packet_count (%d) or max_byte_count (%" G_GINT64_MODIFIER "d/%" G_GINT64_MODIFIER "d) reached",
                      max_packet_count, data_offset, max_byte_count);
        err = 0; /* This is not an error */
        break;
      }
    }

    if (edt) {
      epan_dissect_free(edt);
      edt = NULL;
    }
  }

  wtap_rec_cleanup(&rec);

  if (err != 0 || err_pass1 != 0) {
    tshark_debug("tshark: something failed along the line (%d)", err);
    /*
     * Print a message noting that the read failed somewhere along the line.
     *
     * If we're printing packet data, and the standard output and error are
     * going to the same place, flush the standard output, so everything
     * buffered up is written, and then print a newline to the standard error
     * before printing the error message, to separate it from the packet
     * data.  (Alas, that only works on UN*X; st_dev is meaningless, and
     * the _fstat() documentation at Microsoft doesn't indicate whether
     * st_ino is even supported.)
     */
#ifndef _WIN32
    if (print_packet_info) {
      ws_statb64 stat_stdout, stat_stderr;

      if (ws_fstat64(1, &stat_stdout) == 0 && ws_fstat64(2, &stat_stderr) == 0) {
        if (stat_stdout.st_dev == stat_stderr.st_dev &&
            stat_stdout.st_ino == stat_stderr.st_ino) {
          fflush(stdout);
          fprintf(stderr, "\n");
        }
      }
    }
#endif
    if (err_pass1 != 0) {
      /* Error on pass 1 of two-pass processing. */
      cfile_read_failure_message("TShark", cf->filename, err_pass1,
                                 err_info_pass1);
    }
    if (err != 0) {
      /* Error on pass 2 of two-pass processing or on the only pass of
         one-pass processing. */
      cfile_read_failure_message("TShark", cf->filename, err, err_info);
    }
    success = FALSE;
  }
  if (save_file != NULL) {
    if (pdh && out_file_name_res) {
      if (!wtap_dump_set_addrinfo_list(pdh, get_addrinfo_list())) {
        cmdarg_err("The file format \"%s\" doesn't support name resolution information.",
                   wtap_file_type_subtype_short_string(out_file_type));
      }
    }
    /* Now close the capture file. */
    if (!wtap_dump_close(pdh, &err)) {
      cfile_close_failure_message(save_file, err);
      success = FALSE;
    }
  } else {
    if (print_packet_info) {
      if (!write_finale()) {
        show_print_file_io_error(errno);
        success = FALSE;
      }
    }
  }

out:
  wtap_close(cf->provider.wth);
  cf->provider.wth = NULL;

  wtap_block_array_free(shb_hdrs);
  wtap_block_array_free(nrb_hdrs);

  return success;
}

static gboolean
process_packet_single_pass(capture_file *cf, epan_dissect_t *edt, gint64 offset,
                           wtap_rec *rec, const guchar *pd,
                           guint tap_flags)
{
  frame_data      fdata;
  column_info    *cinfo;
  gboolean        passed;

  /* Count this packet. */
  cf->count++;

  /* If we're not running a display filter and we're not printing any
     packet information, we don't need to do a dissection. This means
     that all packets can be marked as 'passed'. */
  passed = TRUE;

  frame_data_init(&fdata, cf->count, rec, offset, cum_bytes);

  /* If we're going to print packet information, or we're going to
     run a read filter, or we're going to process taps, set up to
     do a dissection and do so.  (This is the one and only pass
     over the packets, so, if we'll be printing packet information
     or running taps, we'll be doing it here.) */
  if (edt) {
    if (print_packet_info && (gbl_resolv_flags.mac_name || gbl_resolv_flags.network_name ||
        gbl_resolv_flags.transport_name))
      /* Grab any resolved addresses */
      host_name_lookup_process();

    /* If we're running a filter, prime the epan_dissect_t with that
       filter. */
    if (cf->dfcode)
      epan_dissect_prime_with_dfilter(edt, cf->dfcode);

    /* This is the first and only pass, so prime the epan_dissect_t
       with the hfids postdissectors want on the first pass. */
    prime_epan_dissect_with_postdissector_wanted_hfids(edt);

    col_custom_prime_edt(edt, &cf->cinfo);

    /* We only need the columns if either
         1) some tap needs the columns
       or
         2) we're printing packet info but we're *not* verbose; in verbose
            mode, we print the protocol tree, not the protocol summary.
       or
         3) there is a column mapped as an individual field */
    if ((tap_flags & TL_REQUIRES_COLUMNS) || (print_packet_info && print_summary) || output_fields_has_cols(output_fields))
      cinfo = &cf->cinfo;
    else
      cinfo = NULL;

    frame_data_set_before_dissect(&fdata, &cf->elapsed_time,
                                  &cf->provider.ref, cf->provider.prev_dis);
    if (cf->provider.ref == &fdata) {
      ref_frame = fdata;
      cf->provider.ref = &ref_frame;
    }

    if (dissect_color) {
      color_filters_prime_edt(edt);
      fdata.flags.need_colorize = 1;
    }

    epan_dissect_run_with_taps(edt, cf->cd_t, rec,
                               frame_tvbuff_new(&cf->provider, &fdata, pd),
                               &fdata, cinfo);

    /* Run the filter if we have it. */
    if (cf->dfcode)
      passed = dfilter_apply_edt(cf->dfcode, edt);
  }

  if (passed) {
    frame_data_set_after_dissect(&fdata, &cum_bytes);

    /* Process this packet. */
    if (print_packet_info) {
      /* We're printing packet information; print the information for
         this packet. */
      g_assert(edt);
      print_packet(cf, edt);

      /* If we're doing "line-buffering", flush the standard output
         after every packet.  See the comment above, for the "-l"
         option, for an explanation of why we do that. */
      if (line_buffered)
        fflush(stdout);

      if (ferror(stdout)) {
        show_print_file_io_error(errno);
        exit(2);
      }
    }

    /* this must be set after print_packet() [bug #8160] */
    prev_dis_frame = fdata;
    cf->provider.prev_dis = &prev_dis_frame;
  }

  prev_cap_frame = fdata;
  cf->provider.prev_cap = &prev_cap_frame;

  if (edt) {
    epan_dissect_reset(edt);
    frame_data_destroy(&fdata);
  }
  return passed;
}

static gboolean
write_preamble(capture_file *cf)
{
  switch (output_action) {

  case WRITE_TEXT:
    return print_preamble(print_stream, cf->filename, get_ws_vcs_version_info());

  case WRITE_XML:
    if (print_details)
      write_pdml_preamble(stdout, cf->filename);
    else
      write_psml_preamble(&cf->cinfo, stdout);
    return !ferror(stdout);

  case WRITE_FIELDS:
    write_fields_preamble(output_fields, stdout);
    return !ferror(stdout);

  case WRITE_JSON:
  case WRITE_JSON_RAW:
    write_json_preamble(stdout);
    return !ferror(stdout);

  case WRITE_EK:
    return !ferror(stdout);

  default:
    g_assert_not_reached();
    return FALSE;
  }
}

static char *
get_line_buf(size_t len)
{
  static char   *line_bufp    = NULL;
  static size_t  line_buf_len = 256;
  size_t         new_line_buf_len;

  for (new_line_buf_len = line_buf_len; len > new_line_buf_len;
       new_line_buf_len *= 2)
    ;
  if (line_bufp == NULL) {
    line_buf_len = new_line_buf_len;
    line_bufp = (char *)g_malloc(line_buf_len + 1);
  } else {
    if (new_line_buf_len > line_buf_len) {
      line_buf_len = new_line_buf_len;
      line_bufp = (char *)g_realloc(line_bufp, line_buf_len + 1);
    }
  }
  return line_bufp;
}

static inline void
put_string(char *dest, const char *str, size_t str_len)
{
  memcpy(dest, str, str_len);
  dest[str_len] = '\0';
}

static inline void
put_spaces_string(char *dest, const char *str, size_t str_len, size_t str_with_spaces)
{
  size_t i;

  for (i = str_len; i < str_with_spaces; i++)
    *dest++ = ' ';

  put_string(dest, str, str_len);
}

static inline void
put_string_spaces(char *dest, const char *str, size_t str_len, size_t str_with_spaces)
{
  size_t i;

  memcpy(dest, str, str_len);
  for (i = str_len; i < str_with_spaces; i++)
    dest[i] = ' ';

  dest[str_with_spaces] = '\0';
}

static gboolean
print_columns(capture_file *cf, const epan_dissect_t *edt)
{
  char   *line_bufp;
  int     i;
  size_t  buf_offset;
  size_t  column_len;
  size_t  col_len;
  col_item_t* col_item;
  gchar str_format[11];
  const color_filter_t *color_filter = NULL;

  line_bufp = get_line_buf(256);
  buf_offset = 0;
  *line_bufp = '\0';

  if (dissect_color)
    color_filter = edt->pi.fd->color_filter;

  for (i = 0; i < cf->cinfo.num_cols; i++) {
    col_item = &cf->cinfo.columns[i];
    /* Skip columns not marked as visible. */
    if (!get_column_visible(i))
      continue;
    switch (col_item->col_fmt) {
    case COL_NUMBER:
      column_len = col_len = strlen(col_item->col_data);
      if (column_len < 5)
        column_len = 5;
      line_bufp = get_line_buf(buf_offset + column_len);
      put_spaces_string(line_bufp + buf_offset, col_item->col_data, col_len, column_len);
      break;

    case COL_CLS_TIME:
    case COL_REL_TIME:
    case COL_ABS_TIME:
    case COL_ABS_YMD_TIME:  /* XXX - wider */
    case COL_ABS_YDOY_TIME: /* XXX - wider */
    case COL_UTC_TIME:
    case COL_UTC_YMD_TIME:  /* XXX - wider */
    case COL_UTC_YDOY_TIME: /* XXX - wider */
      column_len = col_len = strlen(col_item->col_data);
      if (column_len < 10)
        column_len = 10;
      line_bufp = get_line_buf(buf_offset + column_len);
      put_spaces_string(line_bufp + buf_offset, col_item->col_data, col_len, column_len);
      break;

    case COL_DEF_SRC:
    case COL_RES_SRC:
    case COL_UNRES_SRC:
    case COL_DEF_DL_SRC:
    case COL_RES_DL_SRC:
    case COL_UNRES_DL_SRC:
    case COL_DEF_NET_SRC:
    case COL_RES_NET_SRC:
    case COL_UNRES_NET_SRC:
      column_len = col_len = strlen(col_item->col_data);
      if (column_len < 12)
        column_len = 12;
      line_bufp = get_line_buf(buf_offset + column_len);
      put_spaces_string(line_bufp + buf_offset, col_item->col_data, col_len, column_len);
      break;

    case COL_DEF_DST:
    case COL_RES_DST:
    case COL_UNRES_DST:
    case COL_DEF_DL_DST:
    case COL_RES_DL_DST:
    case COL_UNRES_DL_DST:
    case COL_DEF_NET_DST:
    case COL_RES_NET_DST:
    case COL_UNRES_NET_DST:
      column_len = col_len = strlen(col_item->col_data);
      if (column_len < 12)
        column_len = 12;
      line_bufp = get_line_buf(buf_offset + column_len);
      put_string_spaces(line_bufp + buf_offset, col_item->col_data, col_len, column_len);
      break;

    default:
      column_len = strlen(col_item->col_data);
      line_bufp = get_line_buf(buf_offset + column_len);
      put_string(line_bufp + buf_offset, col_item->col_data, column_len);
      break;
    }
    buf_offset += column_len;
    if (i != cf->cinfo.num_cols - 1) {
      /*
       * This isn't the last column, so we need to print a
       * separator between this column and the next.
       *
       * If we printed a network source and are printing a
       * network destination of the same type next, separate
       * them with a UTF-8 right arrow; if we printed a network
       * destination and are printing a network source of the same
       * type next, separate them with a UTF-8 left arrow;
       * otherwise separate them with a space.
       *
       * We add enough space to the buffer for " \xe2\x86\x90 "
       * or " \xe2\x86\x92 ", even if we're only adding " ".
       */
      line_bufp = get_line_buf(buf_offset + 5);
      switch (col_item->col_fmt) {

      case COL_DEF_SRC:
      case COL_RES_SRC:
      case COL_UNRES_SRC:
        switch (cf->cinfo.columns[i+1].col_fmt) {

        case COL_DEF_DST:
        case COL_RES_DST:
        case COL_UNRES_DST:
          g_snprintf(str_format, sizeof(str_format), "%s%s%s", delimiter_char, UTF8_RIGHTWARDS_ARROW, delimiter_char);
          put_string(line_bufp + buf_offset, str_format, 5);
          buf_offset += 5;
          break;

        default:
          put_string(line_bufp + buf_offset, delimiter_char, 1);
          buf_offset += 1;
          break;
        }
        break;

      case COL_DEF_DL_SRC:
      case COL_RES_DL_SRC:
      case COL_UNRES_DL_SRC:
        switch (cf->cinfo.columns[i+1].col_fmt) {

        case COL_DEF_DL_DST:
        case COL_RES_DL_DST:
        case COL_UNRES_DL_DST:
          g_snprintf(str_format, sizeof(str_format), "%s%s%s", delimiter_char, UTF8_RIGHTWARDS_ARROW, delimiter_char);
          put_string(line_bufp + buf_offset, str_format, 5);
          buf_offset += 5;
          break;

        default:
          put_string(line_bufp + buf_offset, delimiter_char, 1);
          buf_offset += 1;
          break;
        }
        break;

      case COL_DEF_NET_SRC:
      case COL_RES_NET_SRC:
      case COL_UNRES_NET_SRC:
        switch (cf->cinfo.columns[i+1].col_fmt) {

        case COL_DEF_NET_DST:
        case COL_RES_NET_DST:
        case COL_UNRES_NET_DST:
          g_snprintf(str_format, sizeof(str_format), "%s%s%s", delimiter_char, UTF8_RIGHTWARDS_ARROW, delimiter_char);
          put_string(line_bufp + buf_offset, str_format, 5);
          buf_offset += 5;
          break;

        default:
          put_string(line_bufp + buf_offset, delimiter_char, 1);
          buf_offset += 1;
          break;
        }
        break;

      case COL_DEF_DST:
      case COL_RES_DST:
      case COL_UNRES_DST:
        switch (cf->cinfo.columns[i+1].col_fmt) {

        case COL_DEF_SRC:
        case COL_RES_SRC:
        case COL_UNRES_SRC:
          g_snprintf(str_format, sizeof(str_format), "%s%s%s", delimiter_char, UTF8_LEFTWARDS_ARROW, delimiter_char);
          put_string(line_bufp + buf_offset, str_format, 5);
          buf_offset += 5;
          break;

        default:
          put_string(line_bufp + buf_offset, delimiter_char, 1);
          buf_offset += 1;
          break;
        }
        break;

      case COL_DEF_DL_DST:
      case COL_RES_DL_DST:
      case COL_UNRES_DL_DST:
        switch (cf->cinfo.columns[i+1].col_fmt) {

        case COL_DEF_DL_SRC:
        case COL_RES_DL_SRC:
        case COL_UNRES_DL_SRC:
          g_snprintf(str_format, sizeof(str_format), "%s%s%s", delimiter_char, UTF8_LEFTWARDS_ARROW, delimiter_char);
          put_string(line_bufp + buf_offset, str_format, 5);
          buf_offset += 5;
          break;

        default:
          put_string(line_bufp + buf_offset, delimiter_char, 1);
          buf_offset += 1;
          break;
        }
        break;

      case COL_DEF_NET_DST:
      case COL_RES_NET_DST:
      case COL_UNRES_NET_DST:
        switch (cf->cinfo.columns[i+1].col_fmt) {

        case COL_DEF_NET_SRC:
        case COL_RES_NET_SRC:
        case COL_UNRES_NET_SRC:
          g_snprintf(str_format, sizeof(str_format), "%s%s%s", delimiter_char, UTF8_LEFTWARDS_ARROW, delimiter_char);
          put_string(line_bufp + buf_offset, str_format, 5);
          buf_offset += 5;
          break;

        default:
          put_string(line_bufp + buf_offset, delimiter_char, 1);
          buf_offset += 1;
          break;
        }
        break;

      default:
        put_string(line_bufp + buf_offset, delimiter_char, 1);
        buf_offset += 1;
        break;
      }
    }
  }

  if (dissect_color && color_filter != NULL)
    return print_line_color(print_stream, 0, line_bufp, &color_filter->fg_color, &color_filter->bg_color);
  else
    return print_line(print_stream, 0, line_bufp);
}

static gboolean
print_packet(capture_file *cf, epan_dissect_t *edt)
{
  if (print_summary || output_fields_has_cols(output_fields))
    /* Just fill in the columns. */
    epan_dissect_fill_in_columns(edt, FALSE, TRUE);

  /* Print summary columns and/or protocol tree */
  switch (output_action) {

  case WRITE_TEXT:
    if (print_summary && !print_columns(cf, edt))
        return FALSE;
    if (print_details) {
      if (!proto_tree_print(print_details ? print_dissections_expanded : print_dissections_none,
                            print_hex, edt, output_only_tables, print_stream))
        return FALSE;
      if (!print_hex) {
        if (!print_line(print_stream, 0, separator))
          return FALSE;
      }
    }
    break;

  case WRITE_XML:
    if (print_summary) {
      write_psml_columns(edt, stdout, dissect_color);
      return !ferror(stdout);
    }
    if (print_details) {
      write_pdml_proto_tree(output_fields, protocolfilter, protocolfilter_flags, edt, &cf->cinfo, stdout, dissect_color);
      printf("\n");
      return !ferror(stdout);
    }
    break;

  case WRITE_FIELDS:
    if (print_summary) {
      /*No non-verbose "fields" format */
      g_assert_not_reached();
    }
    if (print_details) {
      write_fields_proto_tree(output_fields, edt, &cf->cinfo, stdout);
      printf("\n");
      return !ferror(stdout);
    }
    break;

  case WRITE_JSON:
    if (print_summary)
      g_assert_not_reached();
    if (print_details) {
      write_json_proto_tree(output_fields, print_dissections_expanded,
                            print_hex, protocolfilter, protocolfilter_flags,
                            edt, &cf->cinfo, node_children_grouper, stdout);
      return !ferror(stdout);
    }
    break;

  case WRITE_JSON_RAW:
    if (print_summary)
      g_assert_not_reached();
    if (print_details) {
      write_json_proto_tree(output_fields, print_dissections_none, TRUE,
                            protocolfilter, protocolfilter_flags,
                            edt, &cf->cinfo, node_children_grouper, stdout);
      return !ferror(stdout);
    }
    break;

  case WRITE_EK:
    write_ek_proto_tree(output_fields, print_summary, print_hex, protocolfilter,
                        protocolfilter_flags, edt, &cf->cinfo, stdout);
    return !ferror(stdout);
  }

  if (print_hex) {
    if (print_summary || print_details) {
      if (!print_line(print_stream, 0, ""))
        return FALSE;
    }
    if (!print_hex_data(print_stream, edt))
      return FALSE;
    if (!print_line(print_stream, 0, separator))
      return FALSE;
  }
  return TRUE;
}

static gboolean
write_finale(void)
{
  switch (output_action) {

  case WRITE_TEXT:
    return print_finale(print_stream);

  case WRITE_XML:
    if (print_details)
      write_pdml_finale(stdout);
    else
      write_psml_finale(stdout);
    return !ferror(stdout);

  case WRITE_FIELDS:
    write_fields_finale(output_fields, stdout);
    return !ferror(stdout);

  case WRITE_JSON:
  case WRITE_JSON_RAW:
    write_json_finale(stdout);
    return !ferror(stdout);

  case WRITE_EK:
    return !ferror(stdout);

  default:
    g_assert_not_reached();
    return FALSE;
  }
}

void
cf_close(capture_file *cf)
{
  g_free(cf->filename);
}

cf_status_t
cf_open(capture_file *cf, const char *fname, unsigned int type, gboolean is_tempfile, int *err)
{
  wtap  *wth;
  gchar *err_info;

  wth = wtap_open_offline(fname, type, err, &err_info, perform_two_pass_analysis);
  if (wth == NULL)
    goto fail;

  /* The open succeeded.  Fill in the information for this file. */

  /* Create new epan session for dissection. */
  epan_free(cf->epan);
  cf->epan = tshark_epan_new(cf);

  cf->provider.wth = wth;
  cf->f_datalen = 0; /* not used, but set it anyway */

  /* Set the file name because we need it to set the follow stream filter.
     XXX - is that still true?  We need it for other reasons, though,
     in any case. */
  cf->filename = g_strdup(fname);

  /* Indicate whether it's a permanent or temporary file. */
  cf->is_tempfile = is_tempfile;

  /* No user changes yet. */
  cf->unsaved_changes = FALSE;

  cf->cd_t      = wtap_file_type_subtype(cf->provider.wth);
  cf->open_type = type;
  cf->count     = 0;
  cf->drops_known = FALSE;
  cf->drops     = 0;
  cf->snap      = wtap_snapshot_length(cf->provider.wth);
  nstime_set_zero(&cf->elapsed_time);
  cf->provider.ref = NULL;
  cf->provider.prev_dis = NULL;
  cf->provider.prev_cap = NULL;

  cf->state = FILE_READ_IN_PROGRESS;

  wtap_set_cb_new_ipv4(cf->provider.wth, add_ipv4_name);
  wtap_set_cb_new_ipv6(cf->provider.wth, (wtap_new_ipv6_callback_t) add_ipv6_name);

  return CF_OK;

fail:
  cfile_open_failure_message("TShark", fname, *err, err_info);
  return CF_ERROR;
}

static void
show_print_file_io_error(int err)
{
  switch (err) {

  case ENOSPC:
    cmdarg_err("Not all the packets could be printed because there is "
"no space left on the file system.");
    break;

#ifdef EDQUOT
  case EDQUOT:
    cmdarg_err("Not all the packets could be printed because you are "
"too close to, or over your disk quota.");
  break;
#endif

  default:
    cmdarg_err("An error occurred while printing packets: %s.",
      g_strerror(err));
    break;
  }
}

/*
 * General errors and warnings are reported with an console message
 * in TShark.
 */
static void
failure_warning_message(const char *msg_format, va_list ap)
{
  fprintf(stderr, "tshark: ");
  vfprintf(stderr, msg_format, ap);
  fprintf(stderr, "\n");
}

/*
 * Open/create errors are reported with an console message in TShark.
 */
static void
open_failure_message(const char *filename, int err, gboolean for_writing)
{
  fprintf(stderr, "tshark: ");
  fprintf(stderr, file_open_error_message(err, for_writing), filename);
  fprintf(stderr, "\n");
}

/*
 * Read errors are reported with an console message in TShark.
 */
static void
read_failure_message(const char *filename, int err)
{
  cmdarg_err("An error occurred while reading from the file \"%s\": %s.",
             filename, g_strerror(err));
}

/*
 * Write errors are reported with an console message in TShark.
 */
static void
write_failure_message(const char *filename, int err)
{
  cmdarg_err("An error occurred while writing to the file \"%s\": %s.",
             filename, g_strerror(err));
}

static void reset_epan_mem(capture_file *cf,epan_dissect_t *edt, gboolean tree, gboolean visual)
{
  if (!epan_auto_reset || (cf->count < epan_auto_reset_count))
    return;

  fprintf(stderr, "resetting session.\n");

  epan_dissect_cleanup(edt);
  epan_free(cf->epan);

  cf->epan = tshark_epan_new(cf);
  epan_dissect_init(edt, cf->epan, tree, visual);
  cf->count = 0;
}

/*
 * Report additional information for an error in command-line arguments.
 */
static void
failure_message_cont(const char *msg_format, va_list ap)
{
  vfprintf(stderr, msg_format, ap);
  fprintf(stderr, "\n");
}

