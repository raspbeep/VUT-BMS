#!/bin/bash

# Function to check if the required files exist in the zip archive
check_files_in_zip() {
  local zip_file=$1
  local student_login=$2
  local pdf_file="${student_login}.pdf"
  local missing_files=()

  # List of required files
  REQUIRED_FILES=(
    "rds_encoder.cpp"
    "rds_encoder.hpp"
    "rds_decoder.cpp"
    "rds_decoder.hpp"
    "Makefile"
    "$pdf_file"  # PDF file should match the zip file name (student login)
  )

  # Check if the zip file is valid
  unzip -q -l "$zip_file" >/dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo "Error: $zip_file is not a valid zip file or does not exist."
    exit 1
  fi

  # Check if each required file exists in the zip
  for file in "${REQUIRED_FILES[@]}"; do
    if ! unzip -l "$zip_file" | grep -q "$file"; then
      missing_files+=("$file")
    fi
  done

  # Report missing files or confirm everything is present
  if [ ${#missing_files[@]} -eq 0 ]; then
    echo "Directory is in order. All files are present."
  else
    echo "Directory is missing the following files:"
    for file in "${missing_files[@]}"; do
      echo " - $file"
    done
    exit 1
  fi
}

# Check if the zip file name is provided as an argument
if [ $# -eq 0 ]; then
  echo "Usage: $0 <zip-file>"
  exit 1
fi

ZIP_FILE=$1

# Ensure the zip file exists
if [ ! -f "$ZIP_FILE" ]; then
  echo "Error: $ZIP_FILE not found."
  exit 1
fi

# Extract the student login (zip file name without the extension)
STUDENT_LOGIN=$(basename "$ZIP_FILE" .zip)

# Run the check
check_files_in_zip "$ZIP_FILE" "$STUDENT_LOGIN"