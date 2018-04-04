import os
def hp(a):
    s = str()
    for i in range(int(a)//5):
        s = s + '#'
    for i in range(20 - int(a)//5):
        s = s + ' '
    return s

BD_NAME = [ '__Base', 'Shannon', 'Thevenin', 'Norton', 'Von_Neumann', 'Berners_Lee', 'Kuen_Kao', 'Turing', 'Tony_Stark', 'Bool', 'Ohm', 'Mole', 'Monte_Carlo', 'Larry_Roberts', 'Robert_Kahn', 'Musk', 'Hawkin', 'Programmer']
FULL_HP = [10000, 15000, 20000, 25000, 30000, 35000]
HP = [10000, 150, 200, 180, 200, 150, 160, 250, 220, 200, 320, 250, 350, 220, 520, 1000, 360, 100]
if __name__ == "__main__":
    num = input("File Number: ")
    try:
        f = open("turn_save" + str(num) + '.txt', 'r')
    except:
        print('File not exist.')
        exit(0)
    turn_count = 0
    try:
        os.mkdir(os.getcwd() + '\\%s\\'%num)
    except:
        pass
    for line in f:
        turn_count += 1
        false = False 
        true = True
        exec('D=' + line)
        filename = str(turn_count//1000)+str(turn_count//100%10)+str(turn_count//10%10)+str(turn_count%10)
        outf = open(os.getcwd() + "\\%s\\%s.txt"%(num, filename), 'w')
        for i in range(0, 2):
            outf.write('Status %d:\n\tMoney = %8d\tTech = %d\n' % (i, D['status_%d'%i]['money'], D['status_%d'%i]['tech']))
        outf.write('\n')
        for i in range(0, 2):
            outf.write('Base %d:\n\tHP = %8.0f|%s|\n' % (i, D['mainbase_%d'%i]['base_HP'], hp(D['mainbase_%d'%i]['base_HP']/FULL_HP[D['status_%d'%i]['tech']]*100)))
        outf.write('\n')
        for i in range(0, 2):
            outf.write('Unit %d: [%4d]\n' % (i, len(D['unit_%d' % i])))
            for sol in D['unit_%d' % i]:
                outf.write('\t%8d\t%16s\t%6d\t%12s\n'%(sol['id'], sol['name'][12:], sol['hp'], str(sol['pos'])))
        outf.write('\n')
        for i in range(0, 2):
            outf.write('Building %d:\n' % i)
            for typ in D['buildings_%d' % i]:
                outf.write('\t%s: [%4d]\n' % (typ, len(D['buildings_%d' % i][typ])))
                for bd in D['buildings_%d' % i][typ]:
                    outf.write('\t\t%8d\t%12s\t%8.0f|%s|\t%12s\t%4d\n' % (bd['id'], BD_NAME[int(bd['type'])], bd['hp'], hp(100*bd['hp']/(HP[int(bd['type'])] * (1+0.5*bd['level']))), str(bd['pos']), bd['level']))
        outf.write('\n')
        for i in range(0, 2):
            outf.write('Try to Build %d:\n' % i)
            for ins in D['instruments_%d' % i]['construct']:
                outf.write('\t%12s\t%12s\n' % (BD_NAME[int(ins['type'])], str(ins['pos'])))
        outf.close()
        print(turn_count)
