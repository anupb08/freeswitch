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
 * Application.cs -- 
 *
 */
using System;
using System.Collections;
using System.Text;
using System.Runtime.InteropServices;
using FreeSwitch.Marshaling;
using FreeSwitch.Types;
using FreeSwitch.Marshaling.Types;

namespace FreeSwitch.Modules
{
    public class Application
    {
        private string name;
        private string longDescription;
        private string shortDescription;
        private string syntax;
        private ApplicationFunction applicationFunction;
        public HandleRef handle;

        public string Name
        {
            set { name = value; }
            get { return name; }
        }

        public string ShortDescription
        {
            set { shortDescription = value; }
            get { return shortDescription; }
        }

        public string LongDescription
        {
            set { longDescription = value; }
            get { return longDescription; }
        }

        public string Syntax
        {
            set { syntax = value; }
            get { return syntax; }
        }

        public ApplicationFunction ApplicationFunction
        {
            set { applicationFunction = value; }
            get { return applicationFunction; }
        }
    }
}