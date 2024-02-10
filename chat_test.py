#!/usr/bin/python3
from bigdl.llm.transformers import AutoModel
from transformers import AutoTokenizer

model = AutoModel.from_pretrained("./chatglm2-6b", load_in_4bit=True, trust_remote_code=True)
tokenizer = AutoTokenizer.from_pretrained("./chatglm2-6b", trust_remote_code=True)

question = '介绍一下Intel公司'
response_ = ""

for response, history in model.stream_chat(tokenizer, question, history=[]):
    out_text = response.replace(response_, "")
    print(out_text, end="")
    response_ = response

print("\n")



