#ifndef DICL_SOLAR_H_
#define DICL_SOLAR_H_

#include <fstream>
#include <iostream>
#include <cstring>
#include <hdf5.h>

#define H5FILE_ROOT "/home/dicl/R-tree/solar_data/granules"
#define FILELIST_PATH "/home/dicl/R-tree/solar_data/filelist.txt"

typedef struct s1_t {
  int id_tag;
  float yfrac;
  int date;
  float subtanlat;
  float subtanlong;
  int time;
  int ifill;
  float rfill;
  int mid;
} s1_t;

typedef struct s7_t {
  float gm_alt;
  float gp_alt;
  float press;
  float press_u;
  float temp;
  float temp_u;
  float dens;
  float dens_u;
  int flag;
} s7_t;

typedef struct attr3_t {
  int dt;
  float gm_alt;
  float dens;
} attr3_t;

typedef struct mbr3d_t {
  float x1;
  float y1;
  float z1;
  float x2;
  float y2;
  float z2;
} mbr3d_t;

typedef struct kv_t {
  mbr3d_t key;
  char val[100];
} kv_t;

void print_mbr3d(const mbr3d_t &mbr) {
  printf("[(x1: %f, y1: %f, z1: %f), (x2: %f, y2: %f, z2: %f)]\n", mbr.x1, mbr.y1, mbr.z1, mbr.x2, mbr.y2, mbr.z2);
}

void print_kv(const kv_t &kv) {
  printf("Key(MBR):  ");
  print_mbr3d(kv.key);
  printf("Value(filepath):  ");
  printf("%s\n\n", kv.val);
}

herr_t get_section1(hid_t file, s1_t* s1) {
  hid_t grp = H5Gopen2(file, "Section 1.0 - File Header", H5P_DEFAULT);
  hid_t dataset = H5Dopen2(grp, "Section 1.0 - File Header", H5P_DEFAULT);
  
	hid_t s1_tid = H5Tcreate(H5T_COMPOUND, sizeof(s1_t));
	H5Tinsert(s1_tid, "Event Identification Tag", HOFFSET(s1_t, id_tag), H5T_STD_I32LE);
	H5Tinsert(s1_tid, "Year Fraction (YYYY.YYY)", HOFFSET(s1_t, yfrac), H5T_IEEE_F32LE);
	H5Tinsert(s1_tid, "Date (YYYYMMDD)", HOFFSET(s1_t, date), H5T_STD_I32LE);
	H5Tinsert(s1_tid, "SubTangent Lat at 20 Km", HOFFSET(s1_t, subtanlat), H5T_IEEE_F32LE);
	H5Tinsert(s1_tid, "SubTangent Long at 20 Km", HOFFSET(s1_t, subtanlong), H5T_IEEE_F32LE);
	H5Tinsert(s1_tid, "Time (HHMMSS)", HOFFSET(s1_t, time), H5T_STD_I32LE);
	H5Tinsert(s1_tid, "Int Data Fill/Invalid", HOFFSET(s1_t, ifill), H5T_STD_I32LE);
	H5Tinsert(s1_tid, "Real Data Fill/Invalid", HOFFSET(s1_t, rfill), H5T_IEEE_F32LE);
	H5Tinsert(s1_tid, "Mission Id (1 2 or 3)", HOFFSET(s1_t, mid), H5T_STD_I32LE);
	herr_t status = H5Dread(dataset, s1_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, s1);
  return status;
}

herr_t get_section7(hid_t file, s7_t* s7) {
  hid_t grp = H5Gopen2(file, "Section 7.0 - Altitude-based Profiles", H5P_DEFAULT);
  hid_t dataset = H5Dopen2(grp, "Section 7.0 - Altitude-based Profiles", H5P_DEFAULT);

	hid_t s7_tid = H5Tcreate(H5T_COMPOUND, sizeof(s7_t));
	H5Tinsert(s7_tid, "Geometric Altitude (km)", HOFFSET(s7_t, gm_alt), H5T_IEEE_F32LE);
	H5Tinsert(s7_tid, "Geopotential Altitude (km)", HOFFSET(s7_t, gp_alt), H5T_IEEE_F32LE);
	H5Tinsert(s7_tid, "Pressure (hPa)", HOFFSET(s7_t, press), H5T_IEEE_F32LE);
	H5Tinsert(s7_tid, "Pressure Uncertainty (hPa)", HOFFSET(s7_t, press_u), H5T_IEEE_F32LE);
	H5Tinsert(s7_tid, "Temperature (K)", HOFFSET(s7_t, temp), H5T_IEEE_F32LE);
	H5Tinsert(s7_tid, "Temperature Uncertainty (K)", HOFFSET(s7_t, temp_u), H5T_IEEE_F32LE);
	H5Tinsert(s7_tid, "Density (cm-3)", HOFFSET(s7_t, dens), H5T_IEEE_F32LE);
	H5Tinsert(s7_tid, "Density Uncertainty (cm-3)", HOFFSET(s7_t, dens_u), H5T_IEEE_F32LE);
	H5Tinsert(s7_tid, "P/T Array Source Flag (0-4)", HOFFSET(s7_t, flag), H5T_STD_I32LE);
	herr_t status = H5Dread(dataset, s7_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, s7);
  return status;
}

hid_t oepn_file(char* filename) {
  hid_t file = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
  return file;
}

const attr3_t get_attr3(const s1_t* s1, const s7_t* s7) {
  int d = s1->date;
  int t = s1->time;
  unsigned int a = (unsigned int)(d >= 0 ? 2 * (int)d : -2 * (int)d - 1);
  unsigned int b = (unsigned int)(t >= 0 ? 2 * (int)t : -2 * (int)t - 1);
  int c = (int)((a >= b ? a * a + a + b : a + b * b) / 2);
  int dt = d < 0 && t < 0 || d >= 0 && t >= 0 ? c : -c - 1;

  attr3_t attr3;
  attr3.dt = dt;
  attr3.gm_alt = s7->gm_alt;
  attr3.dens = s7->dens;
  return attr3;
}

void get_kv_arr(const char* filepath, kv_t* kv_arr) {
  hid_t file = H5Fopen(filepath, H5F_ACC_RDONLY, H5P_DEFAULT);
  herr_t ret;
  s1_t s1;
  s7_t s7[200];
  get_section1(file, &s1);
  get_section7(file, s7);
  for (int i = 0; i < 200; i+=2) {
    attr3_t p1 = get_attr3(&s1, &s7[i]);
    attr3_t p2 = get_attr3(&s1, &s7[i+1]);
    //soojeong
    mbr3d_t mbr;
    if(p1.dens > p2.dens){
        mbr = {(float)p1.dt, p1.gm_alt, p2.dens, (float)p2.dt, p2.gm_alt, p1.dens};
    }else{
        mbr = {(float)p1.dt, p1.gm_alt, p1.dens, (float)p2.dt, p2.gm_alt, p2.dens};
    }
    kv_arr[i/2].key = mbr;
    strcpy(kv_arr[i/2].val, filepath);
  }
    ret = H5Fclose(file);
}

void get_kv_narr(int n, kv_t* kv_arr) {
  kv_t *curr = kv_arr;
  //int arr_size = 100 * n;
  //kv_arr = (kv_t*)malloc(sizeof(kv_t) * arr_size);
  std::ifstream openFile(FILELIST_PATH);
  int cnt = 0;
  if (openFile.is_open()) {
    std::string line;
    while(getline(openFile, line)) {
      std::string filepath(H5FILE_ROOT);
      filepath.append("/");
      filepath.append(line);

      get_kv_arr(filepath.data(), curr);

      cnt++;

      if (cnt == n) {
        break;
      }
      curr += 100;
    }
    openFile.close();
  }
}
#endif  // DICL_SOLAR_H_
