import base64

root='''
import base64

'''

with open('qgen_src.py','r',encoding='utf-8') as f:
    s = f.read()
    out = base64.b64encode(s.encode())
    with open('qgen.py','w',encoding='utf-8',newline='')as f2:
        f2.write('#THIS PROGRAM IS ENCRYPTED BY qiufuy\n#DO NOT TRY TO REVERSE ENGINEERING!\nimport base64\nlll11ll=exec\nlll11l1=base64.b64decode\nout="'+out.decode()+'"\nlll11ll(lll11l1(out).decode())')
