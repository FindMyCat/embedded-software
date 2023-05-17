/*
 * Copyright 2021-2022 Qorvo US, Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef QORVO_UWBMAC_LOG_H_
#define QORVO_UWBMAC_LOG_H_

#ifndef LOG_TAG
#define LOG_TAG NULL
#endif

#if defined(__ANDROID__)
#include <log/log_main.h>

#define QLOGI(...) IF_ALOGI() ALOGI(__VA_ARGS__)
#define QLOGE(...) IF_ALOGE() ALOGE(__VA_ARGS__)
#define QLOGW(...) IF_ALOGW() ALOGW(__VA_ARGS__)
#define QLOGD(...) IF_ALOGD() ALOGD(__VA_ARGS__)

#elif defined(__linux__)
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

enum qorvo_log_levels {
	LOG_LEVEL_DEBUG = 3,
	LOG_LEVEL_INFORMATION = 4,
	LOG_LEVEL_WARNING = 6,
	LOG_LEVEL_ERROR = 7,
};

extern enum qorvo_log_levels qorvo_log_level;

static inline bool qorvo_test_log(enum qorvo_log_levels level, const char *tag)
{
	(void)tag;
	/*
	On Android, the log level is tested tag by tag.
	cf. https://developer.android.com/reference/android/util/Log#isLoggable(java.lang.String,%20int)

	    The default level of any tag is set to INFO. This means that
	    any level above and including INFO will be logged. Before you
	    make any calls to a logging method you should check to see if
	    your tag should be logged. You can change the default level by
	    setting a system property: 'setprop log.tag.<YOUR_LOG_TAG>
	    <LEVEL>' Where level is either VERBOSE, DEBUG, INFO, WARN,
	    ERROR, or ASSERT. You can also create a local.prop file that
	    with the following in it: 'log.tag.<YOUR_LOG_TAG>=<LEVEL>' and
	    place that in /data/local.prop.

	On Linux, we could test for environment variables, eg.:
	    LOG_TAG_FOOBAR=INFO
	    LOG_TAG_QUUX=DEBUG
	*/
	return (qorvo_log_level <= level);
}

void qorvo_log_print(const char *prio, const char *tag, const char *fmt, ...);

#define QLOGE(FORMAT, ...)                                        \
	do {                                                      \
		if (qorvo_test_log(LOG_LEVEL_ERROR, LOG_TAG))     \
			qorvo_log_print("ERROR", LOG_TAG, FORMAT, \
					##__VA_ARGS__);           \
	} while (0)

#define QLOGW(FORMAT, ...)                                       \
	do {                                                     \
		if (qorvo_test_log(LOG_LEVEL_WARNING, LOG_TAG))  \
			qorvo_log_print("WARN", LOG_TAG, FORMAT, \
					##__VA_ARGS__);          \
	} while (0)

#ifndef NDEBUG

#define QLOGI(FORMAT, ...)                                          \
	do {                                                        \
		if (qorvo_test_log(LOG_LEVEL_INFORMATION, LOG_TAG)) \
			qorvo_log_print("INFO", LOG_TAG, FORMAT,    \
					##__VA_ARGS__);             \
	} while (0)

#define QLOGD(FORMAT, ...)                                        \
	do {                                                      \
		if (qorvo_test_log(LOG_LEVEL_DEBUG, LOG_TAG))     \
			qorvo_log_print("DEBUG", LOG_TAG, FORMAT, \
					##__VA_ARGS__);           \
	} while (0)

#else

#define QLOGI(...) \
	do {       \
	} while (0)

#define QLOGD(...) \
	do {       \
	} while (0)

#endif

/* Zephyr */
#elif defined(__ZEPHYR__) && defined(CONFIG_LOG_PRINTK)
#include <logging/log.h>

#define CUSTOM_PRINTK(prio, tag, fmt, ...)                                   \
	do {                                                                 \
		LOG_PRINTK("%s\t%s:\t" #fmt "\n", prio, tag, ##__VA_ARGS__); \
	} while (0)

#define QLOGI(...) CUSTOM_PRINTK("INF", LOG_TAG, __VA_ARGS__)
#define QLOGE(...) CUSTOM_PRINTK("ERR", LOG_TAG, __VA_ARGS__)
#define QLOGW(...) CUSTOM_PRINTK("WRN", LOG_TAG, __VA_ARGS__)
#define QLOGD(...) CUSTOM_PRINTK("DBG", LOG_TAG, __VA_ARGS__)

#else

#define QLOGI(...) \
	do {       \
	} while (0)
#define QLOGE(...) \
	do {       \
	} while (0)
#define QLOGW(...) \
	do {       \
	} while (0)
#define QLOGD(...) \
	do {       \
	} while (0)

#endif

#endif // QORVO_UWBMAC_LOG_H_
