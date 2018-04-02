import os
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
    data = [0 for i in range(1001)]
    for line in f:
        turn_count += 1
        false = False
        true = True
        #print(line)
        exec(('data[%d]=' % turn_count) + line)
        filename = str(turn_count//1000)+str(turn_count//100%10)+str(turn_count//10%10)+str(turn_count%10)
        outf = open(os.getcwd() + "\\%s\\%s.txt"%(num, filename), 'w')
        D = data[turn_count]
        for i in range(0, 2):
            outf.write('Status %d:\n\tMoney = %8d\tTech = %d\n' % (i, D['status_%d'%i]['money'], D['status_%d'%i]['tech']))
        outf.write('\n')
        for i in range(0, 2):
            outf.write('Base %d:\n\tHP = %8f\n' % (i, D['mainbase_%d'%i]['base_HP']))
        outf.write('\n')
        for i in range(0, 2):
            outf.write('Unit %d:\n' % i)
            for sol in D['unit_%d' % i]:
                outf.write('\t%8d\t%16s\t%6d\t%12s\n'%(sol['id'], sol['name'][12:], sol['hp'], str(sol['pos'])))
        outf.write('\n')
        for i in range(0, 2):
            outf.write('Building %d:\n' % i)
            for typ in D['buildings_%d' % i]:
                outf.write('\t%s:\n' % typ)
                for bd in D['buildings_%d' % i][typ]:
                    outf.write('\t\t%8d\t%8s\t%8.0f\t%12s\t%4d\n' % (bd['id'], bd['type'], bd['hp'], str(bd['pos']), bd['level']))
        outf.write('\n')
        outf.close()
        print(turn_count)
