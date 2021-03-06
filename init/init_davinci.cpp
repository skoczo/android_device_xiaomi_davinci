/*
   Copyright (c) 2015, The Linux Foundation. All rights reserved.
   Copyright (C) 2016 The CyanogenMod Project.
   Copyright (C) 2019-2020 The LineageOS Project.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.
   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <fstream>
#include <unistd.h>
#include <vector>

#include <android-base/logging.h>
#include <android-base/properties.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "property_service.h"
#include "vendor_init.h"

static const char *prop_key[] =
        { "ro.boot.vbmeta.device_state", "ro.boot.verifiedbootstate", "ro.boot.flash.locked",
          "ro.boot.veritymode", "ro.boot.warranty_bit", "ro.warranty_bit",
          "ro.debuggable", "ro.secure", "ro.build.type", 
          "ro.build.tags", "ro.vendor.boot.warranty_bit", "ro.vendor.warranty_bit",
          "vendor.boot.vbmeta.device_state", "vendor.boot.verifiedbootstate", 
          nullptr };

static const char *prop_val[] =
        { "locked", "green", "1",
          "enforcing", "0", "0",
          "0", "1", "user", 
          "release-keys", "0", "0",
          "locked", "green", 
          nullptr };

using android::base::GetProperty;
using android::init::property_set;

constexpr const char *RO_PROP_SOURCES[] = {
    nullptr,   "product.", "product_services.", "odm.",
    "vendor.", "system.",  "bootimage.",
};

constexpr const char *PRODUCTS[] = {
    "davinci",
    "davincin",
};

constexpr const char *DEVICES[] = {
    "Mi 9T",
    "Redmi K20",
};

constexpr const char *BUILD_DESCRIPTION[] = {
    "davinci_eea-user 10 QKQ1.190825.002 V12.0.4.0.QFJEUXM release-keys",
    "davinci_eea-user 10 QKQ1.190825.002 V12.0.4.0.QFJEUXM release-keys",
    "davinciin-user 10 QKQ1.190825.002 V12.0.0.2.QFJINXM release-keys",
};

constexpr const char *BUILD_FINGERPRINT[] = {
    "Xiaomi/davinci/davinci:10/QKQ1.190825.002/V12.0.3.0.QFJCNXM:user/"
    "release-keys",
    "Xiaomi/davinci_eea/davinci:10/QKQ1.190825.002/V12.0.4.0.QFJEUXM:user/"
    "release-keys",
    "Xiaomi/davinciin/davinciin:10/QKQ1.190825.002/V12.0.0.2.QFJINXM:user/"
    "release-keys",
};

constexpr const char *CLIENT_ID[] = {
    "android-xiaomi",
    "android-xiaomi-rev1",
};


void property_override(char const prop[], char const value[], bool add = true) {
  prop_info *pi;

  pi = (prop_info *)__system_property_find(prop);
  if (pi) {
    int result = __system_property_update(pi, value, strlen(value));
    if(result != 0) {
    	LOG(ERROR) << "Error during setting property " << prop;
    } else {
	    LOG(INFO) << "Success during setting property " << prop;
    }
  } else if (add) {
    int result = __system_property_add(prop, strlen(prop), value, strlen(value));
	if(result != 0) {
    	LOG(ERROR) << "Error during setting property " << prop;
    } else {
	    LOG(INFO) << "Success during setting property " << prop;
    }
  }
}


void property_override_new(char const prop[], char const value[]) {
  prop_info *pi;

  pi = (prop_info *)__system_property_find(prop);
  if (pi) {
    int result = __system_property_update(pi, value, strlen(value));
    if(result != 0) {
    	LOG(ERROR) << "Error during setting property " << prop;
    } else {
	    LOG(INFO) << "Success during setting property " << prop;
    }
  } else {
    LOG(ERROR) << "Not set property " << prop;
  }
}


void load_props(const char *model, bool is_9t = false, bool is_in = false) {
  const auto ro_prop_override = [](const char *source, const char *prop,
                                   const char *value, bool product) {
    std::string prop_name = "ro.";

    if (product)
      prop_name += "product.";
    if (source != nullptr)
      prop_name += source;
    if (!product)
      prop_name += "build.";
    prop_name += prop;

    property_override(prop_name.c_str(), value);
  };

  for (const auto &source : RO_PROP_SOURCES) {
    ro_prop_override(source, "device", is_in ? PRODUCTS[1] : PRODUCTS[0], true);
    ro_prop_override(source, "model", model, true);
    if (!is_in) {
      ro_prop_override(source, "name", PRODUCTS[0], true);
      ro_prop_override(source, "fingerprint",
                       is_9t ? BUILD_FINGERPRINT[1] : BUILD_FINGERPRINT[0],
                       false);
      ro_prop_override(nullptr, "fingerprint",
                       is_9t ? BUILD_FINGERPRINT[1] : BUILD_FINGERPRINT[0],
                       false);
    } else {
      ro_prop_override(source, "name", PRODUCTS[1], true);
      ro_prop_override(source, "fingerprint", BUILD_FINGERPRINT[2], false);
      ro_prop_override(nullptr, "fingerprint", BUILD_FINGERPRINT[2], false);
    }
  }
  if (!is_in) {
    ro_prop_override(nullptr, "description",
                     is_9t ? BUILD_DESCRIPTION[1] : BUILD_DESCRIPTION[0],
                     false);
    property_override("ro.boot.product.hardware.sku", PRODUCTS[0]);
  } else {
    ro_prop_override(nullptr, "description", BUILD_DESCRIPTION[2], false);
  }
  ro_prop_override(nullptr, "product", model, false);

  if (is_9t) {
    ro_prop_override(nullptr, "com.google.clientidbase", CLIENT_ID[0], false);
  } else if (is_in) {
    ro_prop_override(nullptr, "com.google.clientidbase", CLIENT_ID[0], false);
    ro_prop_override(nullptr, "com.google.clientidbase.ms", CLIENT_ID[1],
                     false);
  }
  
  for (int i = 0; prop_key[i]; ++i) {
  	// ro override??
  	// ro_prop_override
    property_override(prop_key[i], prop_val[i], true);
  }
	ro_prop_override(nullptr, "ro.boot.vbmeta.device_state", "locked", false);
	ro_prop_override(nullptr, "ro.boot.verifiedbootstate", "green", false);
	ro_prop_override(nullptr, "ro.boot.flash.locked", "1", false);
	ro_prop_override(nullptr, "ro.boot.veritymode", "enforcing", false);
	ro_prop_override(nullptr, "ro.boot.warranty_bit", "0", false);
	ro_prop_override(nullptr, "ro.warranty_bit", "0", false);
	ro_prop_override(nullptr, "ro.debuggable", "0", false);
	ro_prop_override(nullptr, "ro.secure", "1", false);
	ro_prop_override(nullptr, "ro.build.type", "user", false);
	ro_prop_override(nullptr, "ro.build.tags", "release-keys", false);
	ro_prop_override(nullptr, "ro.vendor.boot.warranty_bit", "0", false);
	ro_prop_override(nullptr, "ro.vendor.warranty_bit", "0", false);
	property_override("vendor.boot.vbmeta.device_state", "locked", true);
	property_override("vendor.boot.verifiedbootstate", "green", true);
	
	//ro_prop_override(nullptr, "ro.build.version.security_patch", "2021-01-01", false);
	
    property_override_new("ro.build.version.security_patch", "2021-01-01");
    
//    __system_property_update(pi, value, strlen(value))
	
}

void vendor_load_properties() {
  std::string region;
  region = GetProperty("ro.boot.hwc", "");

  if (region == "CN") {
    load_props(DEVICES[1], false, false);
  } else if (region == "INDIA") {
    load_props(DEVICES[1], false, true);
  } else if (region == "GLOBAL") {
    load_props(DEVICES[0], true, false);
  }
}
