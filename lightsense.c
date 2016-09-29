// -*- mode: c -*-
// lightsense.c
//

#include <mach/mach.h>
#include <inttypes.h>
#import <IOKit/IOKitLib.h>
#import <CoreFoundation/CoreFoundation.h>
#import <emacs-module.h>

static io_connect_t dataPort = 0;

int plugin_is_GPL_compatible;


/* Wrapper around the getLightSensorReadings() function to return values from */
/* light sensors. */
static emacs_value
Flightsense_values(emacs_env *env, ptrdiff_t nargs, emacs_value *Sfun, const char *name) {
    uint64_t values[2];

    if (getLightSensorReadings(values) == 1) {
        return env->make_integer(env, values[0]);
    }

    return env->make_integer(env, 0);
}


/* Gets values from light sensors using OSX Frameworks*/
int getLightSensorReadings(uint64_t values[]) {
    kern_return_t kr;
    io_service_t serviceObject;
    uint32_t outputs = 2;

    serviceObject = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("AppleLMUController"));
    if (!serviceObject) {
        return -1;
    }

    kr = IOServiceOpen(serviceObject, mach_task_self(), 0, &dataPort);
    IOObjectRelease(serviceObject);
    if (kr != KERN_SUCCESS) {
        exit(kr);
    }

    kr = IOConnectCallMethod(dataPort, 0, nil, 0, nil, 0, values, &outputs, nil, 0);
    if (kr == KERN_SUCCESS) {
        return 1;
    }

    return -1;
}


/* The following functions were copied from           */
/* http://diobla.info/blog-archive/modules-tut.html   */
/* They are copied almost verbatim from there.        */
static void bind_function(emacs_env *env, const char *name, emacs_value Sfun) {
    emacs_value Qfset = env->intern(env, "fset");
    emacs_value Qsym = env->intern(env, name);
    emacs_value args[] = { Qsym, Sfun };

    env->funcall(env, Qfset, 2, args);
}


static void provide(emacs_env *env, const char *feature) {
    emacs_value Qfeat = env->intern(env, feature);
    emacs_value Qprovide = env->intern(env, "provide");
    emacs_value args[] = { Qfeat };

    env->funcall(env, Qprovide, 1, args);
}


extern int emacs_module_init(struct emacs_runtime *ert) {
    emacs_env *env = ert->get_environment(ert);

    emacs_value fun = env->make_function(env,
                                         0,
                                         0,
                                         Flightsense_values,
                                         "doc",
                                         NULL);
    bind_function(env, "lightsense-values", fun);
    provide(env, "lightsense");

    return 0;
}
