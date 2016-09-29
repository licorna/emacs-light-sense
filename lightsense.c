// -*- mode: c -*-
// lightsense.c
//
// clang -o lightsense lightsense.c -framework IOKit -framework CoreFoundation
//
// Reference: http://stackoverflow.com/a/18614019/75928
//

#include <mach/mach.h>
#include <inttypes.h>
#import <IOKit/IOKitLib.h>
#import <CoreFoundation/CoreFoundation.h>
#import <emacs-module.h>

static io_connect_t dataPort = 0;

int plugin_is_GPL_compatible;

int getLightSensorReadings(uint64_t[]);
static emacs_value Flightsense_values(emacs_env *, ptrdiff_t nargs, emacs_value*,  const char *);

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
                                         &Flightsense_values,
                                         "doc",
                                         NULL);
    bind_function(env, "lightsense-values", fun);
    provide(env, "lightsense-values");

    return 0;
}

static emacs_value
Flightsense_values(emacs_env *env, ptrdiff_t nargs, emacs_value *Sfun, const char *name) {
    uint64_t values[2];

    if (getLightSensorReadings(values) == 1) {
        return env->make_integer(env, values[0]);
    }

    return 0;
}


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


int main(void) {
    uint64_t light[] = {0, 0};
    int result = 0;

    result = getLightSensorReadings(light);

    if (result == 1) {
        printf("Got ambient light sensors readings: %" PRIu64 " %" PRIu64 "\n", light[0], light[1]);
    }

    return 0;
}
