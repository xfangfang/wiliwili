#!/bin/bash
set -e

# This script is used to request the latest crash report from the NintendoSwitch homebrew FTP server
# Tested with: https://github.com/cathery/sys-ftpd
# Author: https://github.com/xfangfang

if [ $# -lt 2 ]; then
  echo -e "This script is used to request the latest crash report from the NintendoSwitch homebrew FTP server"
  echo -e "Usage: $0 <ftp_config/local> <elf_path> [log_path]"
  echo -e "Example 1: \n\t$0 ftp://user:passwd@192.168.1.3:5000 main.elf"
  echo -e "Example 2: \n\t$0 local main.elf 01693081214_ffffffffffffffff.log"
  exit 1
fi

operation="$1"
elf_path="$2"
log_file="$3"

if [ "$operation" = "local" ]; then
  echo "===>    Load local log file: $log_file"
  crash_report="$(cat "$log_file")"
else
  echo "===>    Using FTP Config: $operation"
  ftp_server="$operation"
  reports_dir="atmosphere/crash_reports"

  echo "===>    Requesting crash report list from $ftp_server/$reports_dir"
  crash_report_path="$(curl -s "$ftp_server"/$reports_dir/ | awk '/\.log$/ {print $9}' | sort -r | head -n 1)"

  echo "===>    Requesting latest report: $ftp_server/$reports_dir/$crash_report_path"
  crash_report="$(curl -s "$ftp_server"/$reports_dir/"$crash_report_path")"
fi

addr2line() {
  aarch64-none-elf-addr2line -e "$elf_path" -f -p -C -a "$1"
}

echo "===>    Exception Info:"
exception_info="$(echo "$crash_report" | awk '/Exception Info:/,/Crashed Thread Info:/ {if (/Crashed Thread Info:/ && ++count>0) {exit} if (!/Exception Info:/) {print}}')"
echo "$exception_info"

echo "===>    LR:"
line="$(echo "$crash_report" | awk '/LR:/ {{print substr($5, 1, length($5)-1)}; exit}')"
addr2line "$line"

echo "===>    PC:"
line="$(echo "$crash_report" | awk '/PC:/ {{print substr($5, 1, length($5)-1)}; exit}')"
addr2line "$line"

echo "===>    Stack info:"
stack_info="$(echo "$crash_report" | awk '/Stack Trace:/,/Stack Dump:/ {if (/Stack Dump:/) {exit} if (!/Stack Trace:/) {print}}')"
echo "$stack_info"
hex_numbers="$(echo "$stack_info" | awk '$5 {print substr($5, 1, length($5)-1)}')"
for item in $hex_numbers; do
  addr2line "$item"
done
