#!/usr/bin/python3
from bigdl.llm.transformers import AutoModel
from transformers import AutoTokenizer


def model_init(model_id):
    model = AutoModel.from_pretrained(model_id, load_in_4bit=True, trust_remote_code=True)
    return model

def tokenizer_init(model_id):
    tokenizer = AutoTokenizer.from_pretrained(model_id, trust_remote_code=True)
    return tokenizer;

def chat_ex(question, model, tokenizer):
    response_ = ""
    out_file = open('chat_out', 'w')
    for response, history in model.stream_chat(tokenizer, question, history=[]):
        out_text = response.replace(response_, "")
        print(out_text, end="", file=out_file)
        out_file.flush()
        response_ = response
    print("\n", file=out_file)
    out_file.close()



