#!/usr/bin/env python
# requires python 2.7 at least

import os
from sys import argv
from time import sleep

import digitalocean

##### GLOBAL VARS #####

key_file_name = '.key'
droplet_prefix = 'nest'
key = None

##### HELPERS #####

def get_key():
    global key
    if key is not None:
        return key

    if not os.path.isfile(key_file_name):
        print 'Please set a key before.'
        exit()

    key_file = open(key_file_name, 'r')
    key = key_file.read()
    key_file.close()
    return key

def get_manager():
    return digitalocean.Manager(token = get_key())

def get_droplets():
    return filter(lambda d: d.name.startswith(droplet_prefix), get_manager().get_all_droplets())

def require_droplets():
    droplets = get_droplets()
    if len(droplets) <= 0:
        print 'No cluster yet'
        exit()
    return droplets

def require_master(droplets = None):
    if droplets is None:
        droplets = require_droplets()
    master = filter(lambda d: d.name.endswith('master'), droplets)
    if len(master) <= 0:
        print 'No master node found.'
        exit()
    return master[0]

def require_slaves(droplets = None):
    if droplets is None:
        droplets = require_droplets()
    slaves = filter(lambda d: not d.name.endswith('master'), droplets)
    if len(slaves) <= 0:
        print 'No slave node(s) found.'
        exit()
    return slaves

def distribute_file(file, droplets = None):
    if droplets is None:
        droplets = require_droplets()

    if not os.path.isfile(file):
        print 'File does not exist yet.'
        exit()

    file_name = os.path.basename(file)

    print 'Distributing file:'
    for drop in require_droplets():
        assert os.system('scp ' + file + ' root@' + drop.ip_address + ':~/' + file_name) == 0

##### COMMANDS #####

def show_help():
    print 'Usage: %s command [args]' % argv[0]
    print 'Only droplet prefixed with \'%s\' are affected by this script.' % droplet_prefix
    print 'Commands:'
    print '- key [token]: setup Digital Ocean token (empty for removing it).'
    print '- list: list current cluster.'
    print '- create [2...] [1gb|2gb|4gb|8gb|16gb]: create n-cluster each having given size using all ssh-key.'
    print '- install [script]: install program executing given bash script on each node.'
    print '- run [1..] [program]: run given python program on the cluster using mpi and n processes.'
    print '- delete: remove current cluster.'
    exit()

def setup_key(key):
    if key == '':
        os.remove(key_file_name)
    else:
        key_file = open(key_file_name, 'w')
        key_file.write(key)
        key_file.close()
        print 'Account set: %s' % get_manager.get_account()

def list_droplets():
    hourly_price = 0.0
    vcpus = 0
    memo = 0
    for drop in require_droplets():
        hourly_price += drop.size['price_hourly']
        vcpus += drop.size['vcpus']
        memo += int(drop.size['slug'][:-2])
        print '%s:\t%svcpu(s)\t%s\t%s\t(private %s)' \
              % (drop.name, drop.size['vcpus'], drop.size['slug'], drop.ip_address, drop.private_ip_address)

    print 'Cluster hourly price ($): %f' % hourly_price
    print 'Cluster total vcpus: %d' % vcpus
    print 'Cluster total memory: %d' % memo

def create_cluster(number, type):

    if number <= 1:
        print 'Number must be greater than 1 (at least one master and one slave).'
        exit()

    if len(get_droplets()) > 0:
        print 'Please delete previous cluster before.'
        exit()

    ssh_keys = get_manager().get_all_sshkeys()
    if len(ssh_keys) <= 0:
        print 'At least one ssh key must be set (under settings/security).'
        exit()

    queue = []
    for i in range(0, number):
        drop = digitalocean.Droplet(token = get_key(),
                                    region = 'ams2',
                                    size_slug = type,
                                    image = 'debian-8-x64',
                                    name = droplet_prefix + ('-master' if i == 0 else '-slave-' + str(i)),
                                    ssh_keys = ssh_keys,
                                    private_networking = True)
        drop.create()
        queue.append(drop.get_actions()[0])

    print 'Spawning droplets:'
    for action in queue:
        print action
        action.wait()

    droplets = require_droplets()
    master = require_master(droplets)
    slaves = require_slaves(droplets)

    print 'Adding to local known hosts.'
    for drop in droplets:
        while os.system('ssh -o StrictHostKeyChecking=no root@' + drop.ip_address + ' hostname > /dev/null 2>&1') != 0:
            sleep(1)

    print 'Generating master key.'
    assert os.system('ssh root@' + master.ip_address + ' \'' +
                     'ssh-keygen -t dsa -N "" -f ~/.ssh/id_dsa;' +
                     'eval `ssh-agent`;' +
                     'ssh-add ~/.ssh/id_dsa;' +
                     '\' > /dev/null 2>&1') == 0

    print 'Distributing master key.'
    for slave in slaves:
        assert os.system('ssh root@' + master.ip_address + ' "cat ~/.ssh/id_dsa.pub" | ssh root@' +
                         slave.ip_address + ' "cat >> ~/.ssh/authorized_keys"') == 0

    print 'Adding slaves to master known hosts.'
    for slave in slaves:
        assert os.system('ssh root@' + master.ip_address + ' \'' +
                         'ssh -o StrictHostKeyChecking=no root@' +
                         slave.private_ip_address + ' hostname' +
                         '\' > /dev/null 2>&1') == 0

    print 'Creating mpi files.'
    private_master_network = 'localhost slots=%s' % master.size['vcpus']
    private_slave_network = map(lambda s: '%s slots=%s' % (s.private_ip_address, s.size['vcpus']), slaves)
    assert os.system('ssh root@' + master.ip_address + ' \'' +
                     'echo "' + private_master_network + '\n\n' + "\n".join(private_slave_network) + '" > cluster.mpi' +
                     '\'') == 0

    print 'Cluster created & configured.'

def install_cluster(script):
    droplets = require_droplets()
    script_name = os.path.basename(script)

    distribute_file(script, droplets)

    print 'Executing installations:'
    for drop in droplets:
        assert os.system('ssh -t -t root@' + drop.ip_address + ' \'' +
                         'nohup bash ' + script_name + ' > ' + script_name+ '.log' +
                         '\' > /dev/null &') == 0

    for drop in droplets:
        while os.system('ssh root@' + drop.ip_address + ' \'' +
                                'lsof | grep ' + script_name + ' > /dev/null' +
                                '\'') != 256:
            print 'Running..'
            sleep(15)

    print 'Install script terminated.'

def run_cluster(pcount, program):

    if pcount < 1:
        print 'Number of process must be positive non-null.'
        exit()

    program_name = os.path.basename(program)
    droplets = require_droplets()
    master = require_master(droplets)
    distribute_file(program, droplets)

    print 'Running from master'
    status = os.system('ssh -t -t root@' + master.ip_address + ' \'' +
                       'source ~/.profile; mpirun -np ' + str(pcount) + ' --mca btl_tcp_if_include eth1 ' +
                       '--hostfile cluster.mpi -x PYTHONPATH python ' + program_name +
                       '\'')
    print 'Success' if status == 0 else 'Failure (status: %d)' % status

def delete_cluster():
    queue = []
    for drop in require_droplets():
        drop.destroy()
        queue.append(drop.get_actions()[0])

    print 'Despawning droplets:'
    for action in queue:
        print action
        action.wait()

    print 'Cluster deleted.'

##### EXECUTION #####

if len(argv) <= 1:
    show_help()

elif argv[1] == 'key':
    setup_key(argv[2] if len(argv) > 2 else '')

elif argv[1] == 'list':
    list_droplets()

elif argv[1] == 'create' and len(argv) == 4:
    create_cluster(int(argv[2]), argv[3])

elif argv[1] == 'install' and len(argv) == 3:
    install_cluster(argv[2])

elif argv[1] == 'run' and len(argv) == 4:
    run_cluster(int(argv[2]), argv[3])

elif argv[1] == 'delete':
    delete_cluster()

else:
    show_help()

