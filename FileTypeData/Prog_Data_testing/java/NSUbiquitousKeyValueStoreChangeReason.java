/*
 * Copyright (C) 2013-2015 RoboVM AB
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.bugvm.apple.foundation;

/*<imports>*/
import java.io.*;
import java.nio.*;
import java.util.*;
import com.bugvm.objc.*;
import com.bugvm.objc.annotation.*;
import com.bugvm.objc.block.*;
import com.bugvm.rt.*;
import com.bugvm.rt.annotation.*;
import com.bugvm.rt.bro.*;
import com.bugvm.rt.bro.annotation.*;
import com.bugvm.rt.bro.ptr.*;
import com.bugvm.apple.corefoundation.*;
import com.bugvm.apple.uikit.*;
import com.bugvm.apple.coretext.*;
import com.bugvm.apple.coreanimation.*;
import com.bugvm.apple.coredata.*;
import com.bugvm.apple.coregraphics.*;
import com.bugvm.apple.coremedia.*;
import com.bugvm.apple.security.*;
import com.bugvm.apple.dispatch.*;
/*</imports>*/

/*<javadoc>*/

/*</javadoc>*/
/*<annotations>*/@Marshaler(ValuedEnum.AsMachineSizedSIntMarshaler.class)/*</annotations>*/
public enum /*<name>*/NSUbiquitousKeyValueStoreChangeReason/*</name>*/ implements ValuedEnum {
    /*<values>*/
    /**
     * @since Available in iOS 5.0 and later.
     */
    ServerChange(0L),
    /**
     * @since Available in iOS 5.0 and later.
     */
    InitialSyncChange(1L),
    /**
     * @since Available in iOS 5.0 and later.
     */
    QuotaViolationChange(2L),
    /**
     * @since Available in iOS 6.0 and later.
     */
    AccountChange(3L);
    /*</values>*/

    private final long n;

    private /*<name>*/NSUbiquitousKeyValueStoreChangeReason/*</name>*/(long n) { this.n = n; }
    public long value() { return n; }
    public static /*<name>*/NSUbiquitousKeyValueStoreChangeReason/*</name>*/ valueOf(long n) {
        for (/*<name>*/NSUbiquitousKeyValueStoreChangeReason/*</name>*/ v : values()) {
            if (v.n == n) {
                return v;
            }
        }
        throw new IllegalArgumentException("No constant with value " + n + " found in " 
            + /*<name>*/NSUbiquitousKeyValueStoreChangeReason/*</name>*/.class.getName());
    }
}
