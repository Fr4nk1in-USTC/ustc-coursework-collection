#!/usr/bin/env python3
import math
import torch
import torch.nn as nn
import torch.nn.functional as F
import matplotlib.pyplot as plt
from typing import List

try:
    from tqdm.auto import tqdm
    use_tqdm = True
except:
    use_tqdm = False

# Hyperparameters
batch_size = 16
block_size = 256  # 256
learning_rate = 1e-3

max_iters = 5000  # training iterations (batchs)

device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

eval_interval = 50  # evaluate loss every 50 iterations
eval_iters = 30  # number of iterations to evaluate loss

# Model setup
n_embd = 64  # embedding dimension
n_heads = 8  # number of attention heads
n_layers = 6  # number of transformer layers


class CharTokenizer:
    """
    a very simple char-based tokenizer. the tokenizer turns a string into a list of integers.
    """

    def __init__(self, corpus: List[str]):
        self.corpus = corpus
        # TODO: calculate the vocab size and create a dictionary that maps each character to a unique integer
        self.n_vocab = len(corpus)
        self.dist = {char: i for i, char in enumerate(corpus)}
        # End of your code

    def encode(self, string: str):
        # TODO: convert a string into a list of integers and return, using the dictionary you created above
        return [self.dist[char] for char in string]
        # End of your code

    def decode(self, codes: List[int]):
        # TODO: convert a list of integers into a string and return, using the dictionary you created above
        return "".join([self.corpus[code] for code in codes])
        # End of your code


class MultiHeadAttention(nn.Module):
    """multi-head self-attention"""

    def __init__(self, n_embd: int, n_heads: int):
        super().__init__()
        # parameters
        if (n_embd % n_heads != 0):
            raise ValueError("n_embd must be divisible by n_heads")

        self.n_heads = n_heads
        self.d_k = n_embd // n_heads

        self.qkv_proj = nn.Linear(n_embd, 3 * n_embd)
        self.out_proj = nn.Linear(n_embd, n_embd)

        # mask for attention
        self.register_buffer("tril",
                             torch.tril(torch.ones(block_size, block_size)))

    def forward(self, inputs: torch.Tensor):
        batch, time, _ = inputs.shape

        qkv = self.qkv_proj(inputs)
        qkv = qkv.reshape(batch, time, self.n_heads, -1)
        qkv = qkv.permute(0, 2, 1, 3)
        q, k, v = qkv.chunk(3, dim=-1)

        out = q @ k.transpose(-2, -1) / math.sqrt(self.d_k)
        out = torch.masked_fill(out,
                                self.tril[:time, :time] == 0,
                                float("-inf"))
        out = F.softmax(out, dim=-1) @ v

        out = out.permute(0, 2, 1, 3).reshape(batch, time, -1)
        out = self.out_proj(out)

        return out


class FeedForward(nn.Module):
    def __init__(self, n_embd: int):
        super().__init__()

        self.net = nn.Sequential(
            nn.Linear(n_embd, 4 * n_embd),
            nn.ReLU(),
            nn.Linear(4 * n_embd, n_embd),
        )

    def forward(self, inputs: torch.Tensor):
        return self.net(inputs)


class Block(nn.Module):
    def __init__(self, n_embd: int, n_heads: int):
        super().__init__()
        self.attn = MultiHeadAttention(n_embd, n_heads)
        self.norm1 = nn.LayerNorm(n_embd)
        self.ff = FeedForward(n_embd)
        self.norm2 = nn.LayerNorm(n_embd)

    def forward(self, inputs: torch.Tensor):
        out = self.attn(inputs)
        out = self.norm1(inputs + out)
        out = self.ff(out)
        out = self.norm2(out + out)
        return out


class Transformer(nn.Module):
    def __init__(self, n_vocab: int, n_embd: int, n_heads: int, n_layers: int):
        super().__init__()
        self.embedding = nn.Embedding(n_vocab, n_embd)
        self.blocks = nn.ModuleList([Block(n_embd, n_heads)
                                     for _ in range(n_layers)])
        self.linear = nn.Linear(n_embd, n_vocab)
        self.softmax = nn.Softmax(dim=2)

    def forward(self, inputs: torch.Tensor, labels=None):
        batch, time = inputs.shape
        embedding = self.embedding(inputs)
        attn = embedding
        for block in self.blocks:
            attn = block(attn)
        logits = self.linear(attn)

        if labels is None:
            loss = None
        else:
            batch, time, channel = logits.shape
            logits = logits.view(batch * time, channel)
            labels = labels.view(batch * time)
            loss = F.cross_entropy(logits, labels)

        return logits, loss

    def generate(self, inputs: torch.Tensor, max_new_tokens: int):
        for _ in range(max_new_tokens):
            logits, _ = self.forward(inputs[:, -block_size:])
            probs = self.softmax(logits)
            next_token = torch.multinomial(probs[:, -1], num_samples=1)
            inputs = torch.cat([inputs, next_token], dim=1)
        return inputs


# Data setup
with open("../data/input.txt", "r", encoding="utf-8") as f:
    text = f.read()
chars = sorted(list(set(text)))

tokenizer = CharTokenizer(chars)
encode = tokenizer.encode
decode = tokenizer.decode
n_vocab = tokenizer.n_vocab

# separate the dataset into train and validation
data = torch.tensor(encode(text), dtype=torch.long)
data_len = data.shape[0]
train_data = data[:-data_len // 10]
val_data = data[-data_len // 10:]

# Unfold into blocks
train_blocks = train_data.unfold(0, block_size, 1)
train_x = train_blocks[:-1]
train_y = train_blocks[1:]

val_blocks = val_data.unfold(0, block_size, 1)
val_x = val_blocks[:-1]
val_y = val_blocks[1:]


def get_batch(split: str):
    "Get batched data"
    if split == "train":
        x = train_x
        y = train_y
    elif split == "val":
        x = val_x
        y = val_y
    else:
        raise ValueError("split must be either 'train' or 'val'")
    idx = torch.randint(x.shape[0], (batch_size,))
    return x[idx].to(device), y[idx].to(device)


# Training setup
@torch.no_grad()
def estimate_loss(model: Transformer):
    out = {}
    model.eval()
    for split in ["train", "val"]:
        losses = torch.zeros(eval_iters)
        for k in range(eval_iters):
            x, y = get_batch(split)
            _, loss = model(x, y)
            losses[k] = loss
        out[split] = losses.mean().item()
    return out


def generate(model: Transformer, text: str = ""):
    if text == "":
        context = torch.zeros((1, 1), dtype=torch.long, device=device)
    else:
        context = torch.tensor([encode(text)], dtype=torch.long, device=device)
    print(decode(model.generate(context, max_new_tokens=500)[0].tolist()))


def train(model: Transformer):
    optimizer = torch.optim.AdamW(model.parameters(), lr=learning_rate)
    model.train()

    train_losses = []
    val_losses = []

    if use_tqdm:
        loop = tqdm(range(max_iters))

        for iter in loop:

            if iter % eval_interval == 0:
                losses = estimate_loss(model)
                train_losses.append(losses["train"])
                val_losses.append(losses["val"])
                loop.set_postfix(last_train_loss=f"{losses['train']: .6f}",
                                 last_val_loss=f"{losses['val']: .6f}")

            inputs, labels = get_batch("train")
            _, loss = model(inputs, labels)
            optimizer.zero_grad(set_to_none=True)
            loss.backward()
            optimizer.step()
    else:
        for iter in range(max_iters):

            if iter % eval_interval == 0:
                losses = estimate_loss(model)
                train_losses.append(losses["train"])
                val_losses.append(losses["val"])
                print(
                    f"step {iter}: train loss {losses['train']:.4f}, val loss {losses['val']:.4f}"
                )

            inputs, labels = get_batch("train")

            _, loss = model(inputs, labels)
            optimizer.zero_grad(set_to_none=True)
            loss.backward()
            optimizer.step()

    return train_losses, val_losses


def plot_losses(train_losses: list[float], val_losses: list[float]):
    xs = [eval_interval * i for i in range(len(train_losses))]
    plt.xlabel("Iteration")
    plt.ylabel("Loss")
    plt.title("Transformer Training Losses")
    plt.plot(xs, train_losses, label="train")
    plt.plot(xs, val_losses, label="val")
    plt.legend()
    plt.show()
    plt.savefig("losses.png")


# Model
model = Transformer(n_vocab, n_embd, n_heads, n_layers).to(device)
train_losses, val_losses = train(model)
plot_losses(train_losses, val_losses)

generate(model, "The meaning of life is")
