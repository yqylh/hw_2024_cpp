def gen_all_paras(para_list):
    if len(para_list) == 1:
        return [[i] for i in para_list[0]]
    else:
        sub_paras = gen_all_paras(para_list[1:])
        result = []
        for para in para_list[0]:
            for sub_para in sub_paras:
                result.append([para] + sub_para)
        return result
    
if __name__ == "__main__":
    para_list = [[160, 150, 130], [0.5, 0.3], [1, 2], [3, 4, 5]]
    print(gen_all_paras(para_list))