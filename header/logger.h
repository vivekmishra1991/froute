/* Logging :: */

/* Default logging level is 0 == No logs. */
/* Log Level = 1 Only WARNs will be displayed in this level.  */
/* Log level = 2 . WARNs and LOG_FLOWs will be showed in this level. */
/* Log level = 3 . All logs will be displayed. */



#ifndef LOG_LEVEL    //Check if log level is set explicitly
#define LOG_LEVEL 0  // Set default logging level
#endif

#define WARN(expr, args...) if (LOG_LEVEL >= 1) fprintf(stdout, expr, ##args)
#define LOG_DEBUG(expr, args...) if (LOG_LEVEL >= 2) fprintf (stderr, expr, ##args)
#define LOGGER(expr, args...) if(LOG_LEVEL >= 3) fprintf(stderr, expr, ##args)
