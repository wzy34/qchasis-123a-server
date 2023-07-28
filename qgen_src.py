#Python 3.x
version = 5
import subprocess
import sys
import requests
import os
try:
    import yaml
except Exception as e:
    print('Detect that pyyaml is NOT installed , now install it!')
    os.system('pip install pyyaml')
    print('Finished, Please restart the program')
    os._exit(0)
def printerr(s):
    print("\033[0;31;40m"+s+"\033[0m")
def printok(s):
    print("\033[0;32;40m"+s+"\033[0m")
def printwarn(s):
    print("\033[0;33;40m"+s+"\033[0m")
headers = {
    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/104.0.0.0 Safari/537.36",
}
def vercode(res):
    if res[0]=='/' and res[1]=='*' and res[2]=='*':
        return res
    return ""
def getpy():
    res = requests.get("https://qiufuyu123.github.io/qchasis-123a-server/qgen.py",headers=headers).content.decode()
    if res[0] == '#':
        return res
    return ''
def getdoth():
    res = requests.get("https://qiufuyu123.github.io/qchasis-123a-server/qchasis.h",headers=headers).content.decode()
    return vercode(res)
def getdotcpp():
    res = requests.get("https://qiufuyu123.github.io/qchasis-123a-server/qchasis.cpp",headers=headers).content.decode()
    return vercode(res)
def getcfg():
    res = requests.get("https://qiufuyu123.github.io/qchasis-123a-server/ver.yaml",headers=headers).content.decode()
    if res[0]=='v' and res[1] == 'e':
        return res
    return ''
def validcfg(s):
    return 'ver' in s and 'wheels' in s and 'left_motors' in s and 'right_motors' in s
def updatesrc():
    print("[RECV] qchasis.h ...",end='',flush=True)
    codeh = getdoth()
    if codeh == '':
        printerr('[FAIL]: Format Received Error!')
        return False
    printok('[DONE]')
    print("[RECV] qchasis.cpp ...",end='',flush=True)
    codecpp = getdotcpp()
    if codecpp == '':
        printerr('[FAIL]: Format Received Error!')
        return False
    printok('[DONE]')
    try:
        with open('qtemplate/qchasis.h.t','w',encoding='utf-8', newline='') as f:
            f.write(codeh)
        with open('qtemplate/qchasis.cpp.t','w',encoding='utf-8',newline='') as f:
            f.write(codecpp)
    except Exception as e:
        printerr('Fail to create(write) file!')
        return False
    return True
def yesorno():
    res = input("[y/n](yes or no):")
    if res == 'y' or res == 'Y':
        return True
    return False
def rmdirf(s):
    for root, dirs, files in os.walk(s, topdown=False):
        for name in files:
            os.remove(os.path.join(root, name))
        for name in dirs:
            os.rmdir(os.path.join(root, name))
    os.rmdir(s)
    
def backup():
    if not os.path.exists('src/qchasis.cpp') or not os.path.exists('include/qchasis.h'):
        return True
    try:
        with open('src/qchasis.cpp','r',newline='',encoding='utf-8') as f:
            ctx = f.read()
            with open('src/qchasis.cpp.bak','w',newline='',encoding='utf-8') as f2:
                f2.write(ctx)
        with open('include/qchasis.h','r',newline='',encoding='utf-8') as f:
            ctx = f.read()
            with open('include/qchasis.h.bak','w',newline='',encoding='utf-8') as f2:
                f2.write(ctx)
        os.remove('src/qchasis.cpp')
        os.remove('include/qchasis.h')
    except Exception as e:
        print(e)
        printerr('Fail to backup file!')
        return False
def createenv():
    printok("Now, start to download the latest template from server")
    try:
        rmdirf('qtemplate/')
    except FileNotFoundError as e:
        printok("Create 'qtemplate' Dir")
    os.mkdir('qtemplate')
    if not updatesrc():
        return False
    print("[RECV] ver.yaml ...",end='',flush=True)
    res = getcfg()
    printok("[DONE]")
    try:
        with open('qtemplate/chasis.yaml','w',newline='') as f:
            f.write(res)
    except Exception as e:
        printerr("Fail to write `chasis.yaml`")
        return False
    cfg = yaml.load(res,yaml.FullLoader)
    print("Validating config ...",end='',flush=True)
    if not validcfg(cfg):
        printerr('Invalid config file format!')
        return False
    printok("[DONE]")
    printwarn("Detected old `qchasis.cpp` and `qchasis.h`, \nTo continue, are you sure to delete them?\nA backup file will be created, do not worry about losing your file.")
    if not yesorno():
        printerr('Operation Abort by user')
        return True
    print('Backup old qchasis files...',end='',flush=True)
    backup()
    printok('[DONE]')
    return True
def checkenv():
    if (not os.path.exists('qtemplate/chasis.yaml')) or \
        (not os.path.exists('qtemplate/qchasis.h.t') or \
        (not os.path.exists('qtemplate/qchasis.cpp.t'))):
        return False
    return True

def prompt():
    printok("---------- Q-gen by qiufuyu ----------")
    print("Arg List:")
    print("  make   :  Generate from template")
    print("  build  :  make & compile use pros")
    print("  update :  Download Latest template from server")
    print("  upload :  ONLY upload")
    print("  updatec:  Update qgen script")
    print("  ver    :  Print version information")

script = '''
@set PATH=%PATH%;%USERPROFILE%\\AppData\\Roaming\\Code\\User\\globalStorage\\sigbots.pros\\install\\pros-cli-windows
@set PROS_TOOLCHAIN=%USERPROFILE%\\AppData\\Roaming\\Code\\User\\globalStorage\\sigbots.pros\\install\\pros-toolchain-windows\\usr
'''

def needupdate():
    try:
        with open('qtemplate/chasis.yaml','r',encoding='utf-8') as f:
            res = f.read()
        res=yaml.load(res,yaml.FullLoader)
        if not validcfg(res):
            print('Bad Format for chasis.yaml!')
            return 0
    except Exception as e:
        printerr('Fail to load chasis.yaml!')
        return 0
    cur = int(res['ver'])
    print('Look up update in server ...',end='',flush= True)
    net = getcfg()
    if net == '':
        printerr('Cannot Download remote config!')
        return 0
    net = yaml.load(net,yaml.FullLoader)
    now = int(net['ver'])
    printok('[DONE]')
    print('Current Version: '+str(cur)+' , Latest Version: '+str(now))
    if now == cur:
        return 0
    return now

def runcmd(cmd):
    print('Generate Auto Compile Script...',end='')
    try:
        with open('qmake.bat','w',encoding='utf-8',newline='') as f:
            f.write(script+cmd)
    except Exception as e:
        printerr('Fail to write Auto Compile Script!')
        return False
    printok('[DONE]')
    print('Wait for PROS CLI response ...')
    if os.system('qmake.bat')!=0:
        printerr('PROS Process ERROR!')
        return False
    else:
        print("Success!")
    print("Clean up environment... ",end='')
    os.remove('qmake.bat')
    return True

def build(ispros):
    backup()
    printwarn('*************** QGEN ***************')
    print    ('* Created By qiufuyu from 123A     *')
    print    ('* Config: chasis.yaml TASK: BUILD  *')
    printwarn('*************** QGEN ***************')
    try:
        with open('qtemplate/chasis.yaml','r',encoding='utf-8') as f:
            cfg = yaml.load(f.read(),yaml.FullLoader)
    except Exception as e:
        printerr('Fail to load chasis.yaml!')
        return False
    if not validcfg(cfg):
        printerr('Bad Format in chasis.yaml!')
        return False
    try:
        with open('qtemplate/qchasis.h.t','r',encoding='utf-8') as f:
            codeh = f.read()
        with open('qtemplate/qchasis.cpp.t','r',encoding='utf-8') as f:
            codecpp = f.read()
    except Exception as e:
        print(e)
        printerr('Fail to load template file')
        return False
    print('Generate `qchasis.h` with `chasis.yaml` ...',end='',flush=True)
    codeh = codeh.replace('{lwf}',str(abs(cfg['left_motors']['front']))).replace('{lwm}',str(abs(cfg['left_motors']['mid']))).replace('{lwb}',str(abs(cfg['left_motors']['back'])))
    codeh = codeh.replace('{rwf}',str(abs(cfg['right_motors']['front']))).replace('{rwm}',str(abs(cfg['right_motors']['mid']))).replace('{rwb}',str(abs(cfg['right_motors']['back'])))
    codecpp = codecpp.replace('{revl1}',str('-' if (cfg['left_motors']['front']<0) else '')).replace('{revl2}',str('-' if (cfg['left_motors']['mid']<0) else '')).replace('{revl3}',str('-' if (cfg['left_motors']['back']<0) else ''))
    codecpp = codecpp.replace('{revr1}',str('-' if (cfg['right_motors']['front']<0) else '')).replace('{revr2}',str('-' if (cfg['right_motors']['mid'])<0 else '')).replace('{revr3}',str('-' if (cfg['right_motors']['back'])<0 else ''))
    codeh = codeh.replace('{revl1}',str('true' if (cfg['left_motors']['front']<0) else 'false')).replace('{revl2}',str('true' if (cfg['left_motors']['mid']<0) else 'false')).replace('{revl3}',str('true' if (cfg['left_motors']['back']<0) else 'false'))
    codeh = codeh.replace('{revr1}',str('true' if (cfg['right_motors']['front']<0) else 'false')).replace('{revr2}',str('true' if (cfg['right_motors']['mid'])<0 else 'false')).replace('{revr3}',str('true' if (cfg['right_motors']['back'])<0 else 'false'))
    codeh = codeh.replace('{gyro}',str(cfg['gyro']))
    wheels = cfg['wheels']
    pids = cfg['pids']
    codeh = codeh.replace('{rpm}',str(wheels['rpm'])).replace('{diameter}',str(wheels['diameter'])).replace('{track}',str(wheels['track']))
    codeh = codeh.replace('{gearnum}','pros::E_MOTOR_GEAR_'+str(wheels['gear']).upper()).replace('{gearcolor}','okapi::AbstractMotor::gearset::'+str(wheels['gear']))
    codeh = codeh.replace('{team}',cfg['team'])
    codeh = codeh.replace('{pids_drive_kp}',str(pids['drive']['kp'])).replace('{pids_drive_kd}',str(pids['drive']['kd'])).replace('{pids_drive_ks}',str(pids['drive']['ks']))
    codeh = codeh.replace('{pids_angle_kp}',str(pids['angle']['kp'])).replace('{pids_angle_kd}',str(pids['angle']['kd'])).replace('{pids_angle_ks}',str(pids['angle']['ks']))
    others = cfg['others']
    codeh = codeh.replace('{shoot_dist}',str(others['shoot_dist']))
    codeh = codeh.replace('{tri_motors}','' if cfg['tri_motors'] else '//').replace('{toward}',str(cfg['toward']))
    modes = cfg['mod']
    codeh = codeh.replace('{need_calib}','false' if modes['drive_test'] else 'true')
    codeh = codeh.replace('{diagno}','true' if modes['diagno'] else 'false')
    printok('[DONE]')
    
    print('Generate `qchasis.cpp` with `chasis.yaml` ...',end='',flush=True)
    codecpp = codecpp.replace('{team}',cfg['team'])
    printok('[DONE]')
    print('Write to file ...',end='',flush=True)
    try:
        with open('src/qchasis.cpp','w',encoding='utf-8',newline='') as f:
            f.write(codecpp)
        with open('include/qchasis.h','w',encoding='utf-8',newline='') as f:
            f.write(codeh)
    except Exception as e:
        printerr('Fail to write to file')
        return False
    printok('[DONE]')
    printok('*************** QGEN FINISHED ***************')
    if not ispros:
        return True
    runcmd('@pros make')
    printok('[DONE]')
    return True

def update(ver):
    if not updatesrc():
        return False
    try:
        with open('qtemplate/chasis.yaml','r+',encoding='utf-8') as f:
            f.seek(0)
            f.write('ver: '+str(ver))
    except Exception as e:
        print(e)
        printerr('Fail to load chasis.yaml!')
        return False
    return True
    
def updatec():
    print("Checking update ...")
    s = getcfg()
    if s=='':
        printerr('Fail to connet to server')
        return False
    s = yaml.load(s,yaml.FullLoader)
    if s['cver'] >version:
        printwarn('HEY! Current `qgen.py` is not UP-TO-DATE!')
        printwarn('Ready to update?')
        if not yesorno():
            return False
        print('[RECV] qgen.py ...',end='',flush=True)
        npy = getpy()
        if npy == '':
            printerr('Bad Format for qgen.py Received!')
            return False
        printok('[DONE]')
        try:
            with open('qgen_new.py','w',newline='',encoding='utf-8') as f:
                f.write(npy)
            with open('qmake.bat','w',encoding='utf-8',newline='')as f:
                f.write('@del qgen.py\n@move qgen_new.py qgen.py\n@del qmake.bat')
        except Exception as e:
            printerr('Fail to write to qgen_new.py')
        print('Execute auto-update script...')
        subprocess.Popen('qmake.bat',creationflags=subprocess.CREATE_NEW_CONSOLE)
        sys.exit()
    print('Not neccessary to update!')
if __name__ == '__main__':
    args = sys.argv
    argc = len(args)
    if argc != 2:
        prompt()
    else:
        cmd = args[1]
        if cmd == 'ver':
            print('Q-gen version: '+str(version)+', author: qiufuyu')
            sys.exit()
        if not checkenv():
            printerr("Qtemplate Environment Not Found in Current Path")
            createenv()
        if cmd == 'build':
            print("Ready to build template ...")
            build(True)
        elif cmd == 'make':
            build(False)
        elif cmd == 'update':
            print("Ready to check version ...")
            v = needupdate()
            if v!=0:
                printwarn('HEY! Current qgen version is not up-to-date!')
                printwarn('Ready to update?')
                if yesorno():
                    if update(v):
                        printok('[DONE]')
            else:
                print('Current is already the latest version!')
        elif cmd == 'upload':
            runcmd('@pros upload')
        elif cmd == 'updatec':
            updatec()