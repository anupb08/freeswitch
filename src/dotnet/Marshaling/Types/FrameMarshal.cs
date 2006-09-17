/* 
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2006, James Martelletti <james@nerdc0re.com>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * James Martelletti <james@nerdc0re.com>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * 
 * James Martelletti <james@nerdc0re.com>
 *
 *
 * FrameMarshal.cs -- 
 *
 */
using System;
using System.Runtime.InteropServices;
using FreeSwitch.Types;

namespace FreeSwitch.Marshaling.Types
{
    [StructLayout(LayoutKind.Sequential)]
    internal class FrameMarshal
    {
        internal IntPtr codec;
        internal IntPtr source;
        internal IntPtr packet;
        internal UInt32 packetlen;
        internal IntPtr data;
        internal UInt32 datalen;
        internal UInt32 buflen;
        internal UInt32 samples;
        internal UInt32 rate;
        internal byte payload;
        internal UInt32 timestamp;
        internal byte flags;
    }
}
