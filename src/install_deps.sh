#!/usr/bin/env bash
#*******************************************************************************
#*   (c) 2018 ZondaX GmbH
#*
#*  Licensed under the Apache License, Version 2.0 (the "License");
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*      http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an "AS IS" BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#********************************************************************************

SCRIPT_DIR=$(cd $(dirname $0) && pwd)

os_string="$(uname -s)"
case "${os_string}" in
	Linux*)
		pip install --user -U setuptools
		pip install --user -U --no-cache ledgerblue ecpy
		;;
	Darwin*)
		brew install libusb
		pip install -U ledgerblue ecpy
		;;
	*)
		echo "OS not recognized"
		;;
esac
