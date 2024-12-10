Import("env")
env.Replace(PROGNAME=f'firmware_{env["PIOENV"]}')
