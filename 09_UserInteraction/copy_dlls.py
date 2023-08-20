# import required module
import glob
import os,sys,shutil

# assign directory
directory = 'files'
 
# iterate over files in
# that directory
for proj_path in glob.iglob(f'./*'):
    debug_path = proj_path + "/x64/Debug"
    if os.path.isdir(debug_path):
        for dll_fullpath in glob.iglob(debug_path + f'/*.dll'):
            
            dll_basename = os.path.basename(dll_fullpath)
            
            target_name = proj_path + "/" + dll_basename

            if os.path.exists(target_name):
                print(target_name + " already exists.")
                continue

            shutil.copyfile(dll_fullpath, target_name)

            print(target_name + " copied.")


