import os

###########################################

def set(key,val):
  os.environ[str(key)] = str(val)

###########################################

def prepend(key,val):
  if False==(str(key) in os.environ):
    set(key,val)
  else:
    os.environ[str(key)] = str(val) + ":" + os.environ[key]

###########################################

def append(key,val):
  if False==(str(key) in os.environ):
    set(key,val)
  else:
    os.environ[str(key)] = os.environ[str(key)]+":"+str(val)
