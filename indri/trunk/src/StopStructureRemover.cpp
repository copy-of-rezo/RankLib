/*==========================================================================
 * Copyright (c) 2012 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */

#include "indri/StopStructureRemover.hpp"

std::string indri::query::StopStructureRemover::transform(std::string query) {
  const char *_structures[] = {
    "you ever had ",
    "why do i ",
    "why do we ",
    "why is there ",
    "why will not ",
    "why will ",
    "why does ",
    "why can ",
    "why are ",
    "why did ",
    "why should ",
    "why is ",
    "why do people think ",
    "why do ",
    "why ",
    "with what is ",
    "where can i find stuff about ",
    "where can i ",
    "where can we ",
    "where can ",
    "where are ",
    "when was ",
    "when is ",
    "when did ",
    "when are ",
    "when ",
    "what are some experiences people ",
    "what are some people s experiences ",
    "what are some people s opinions ",
    "what are some people s opinions about ",
    "what are people s opinions on ",
    "what are people s opinions ",
    "what are people s opinions about ",
    "what are people s saying about ",
    "what are people saying about ",
    "what are the people saying about ",
    "what are people saying ",
    "what will happen ",
    "what would happen if ",
    "what causes ",
    "what can be done about ",
    "what can be done ",
    "what happens ",
    "what makes ",
    "what do people say about ",
    "what do people say ",
    "what do people think about ",
    "what do people think ",
    "what do ",
    "what is the ",
    "what is ",
    "what are the effects ",
    "what are the ",
    "what are ",
    "was the ",
    "true or false a ",
    "theres a ",
    "the ",
    "tell me which is ",
    "tell me stuff about ",
    "still hoping to find a ",
    "some info about ",
    "should we be ",
    "should ",
    "recommend ",
    "please name 5 ",
    "please name ",
    "please explain in detail about ",
    "please compare ",
    "need to find a ",
    "name of the five ",
    "names of ",
    "name the ",
    "name a ",
    "my wife wants to know ",
    "my wife has ",
    "my mother has ",
    "my mom has ",
    "my husband wants to get a ",
    "my girlfriend is fed up of my ",
    "my boyfriend has ",
    "my best friend came down with ",
    "looking for info on an ",
    "looking for a ",
    "list ",
    "know any good ",
    "ive heard that ",
    "is the ",
    "information on how to ",
    "information about the ",
    "information about ",
    "in your opinion what would be the ",
    "in which ",
    "in what ",
    "in the ",
    "in simple terms could some please tell me what ",
    "im looking for an ",
    "im looking for a ",
    "im looking for ",
    "im also interested to ",
    "im 21 and am having ",
    "im ",
    "if you have ",
    "if the ",
    "if some is ",
    "if it is true that ",
    "if i am having a ",
    "i would like to know the ",
    "i would like information about ",
    "i was just told i have a ",
    "i want to understand the ",
    "i want to try ",
    "i want to know what your ",
    "i want to know the meaning of ",
    "i want to know something about ",
    "i want to know if there is any ",
    "i want to know everything about ",
    "i want to know a little about ",
    "i want to be ",
    "i want details of ",
    "i want ",
    "i read that ",
    "i need to know the ",
    "i need to find information on ",
    "i need to find an ",
    "i need to devise a ",
    "i need to ",
    "i need the correct spelling of ",
    "i need some technical information about how a ",
    "i need information on ",
    "i need as many ",
    "i need a list of ",
    "i need a ",
    "i heard that ",
    "i have taken ",
    "i have just been told my aunt has a ",
    "i cant seem to ",
    "i am trying to find an ",
    "how can we have ",
    "how can we ",
    "how much of ",
    "how did ",
    "how much ",
    "how would ",
    "how does one ",
    "how does ",
    "how do people think ",
    "how do i ",
    "how are the ",
    "how are ",
    "how is ",
    "how ",
    "hi im trying to find some ",
    "help me with my ",
    "help im freaked out i might have ",
    "give me information on a ",
    "give me information on ",
    "give me a ",
    "give me ",
    "give a brief explanation ",
    "give 5 examples of ",
    "give ",
    "for a year ive been getting some ",
    "finish the sentence ",
    "find comments about ",
    "find the ",
    "find me ",
    "find a ",
    "discuss the ",
    "describe ",
    "define ",
    "could you please tell me ",
    "could you briefly explain about ",
    "could someone explain the ",
    "could a ",
    "could ",
    "compare the ",
    "compare ",
    "can you think of a ",
    "can you tell me about the ",
    "can you point me the ",
    "can you please give me an example of a ",
    "can you please give ",
    "can you please ",
    "can you name 1 ",
    "can you list ",
    "can you be ",
    "can we ",
    "can u watch ",
    "can u find for me the website about ",
    "can the ",
    "can someone tell me how can i ",
    "can someone please explain about ",
    "can someone please describe what ",
    "can someone help me with my ",
    "can i use ",
    "can i know ",
    "can i have ",
    "can i get ",
    "can i ",
    "can anyone tell me what is ",
    "can anyone tell me what ",
    "can anyone tell me more about ",
    "can anyone suggest a good ",
    "can anyone say what is ",
    "can anyone help with ",
    "can anyone give me info on ",
    "can any one help me out with a ",
    "can any body suggest me ",
    "can a ",
    "can ",
    "at what ",
    "are there ",
    "anywhere to get a ",
    "anyone want to ",
    "anyone out there have ",
    "anyone know where i can ",
    "anyone know what to use for ",
    "anyone know of a ",
    "anyone know ",
    "anyone here know what the ",
    "anyone heard any news of ",
    "anyone have any ",
    "anyone have a ",
    "anyone have ",
    "anyone had a ",
    "anyone familiar with the ",
    "anyone ever heard of the ",
    "anyone ever ",
    "anybody know of ",
    "any tips for ",
    "any opinions on ",
    "any advice on how to heal a ",
    "do ",
    "does ",
    "what ",
    "did ",
    "who is ",
    "who ",
    "is ",
    "has ",
    "were ",
    "will ",
    "should ",
    "which ",
    "have ",
    0};

  int i = 0;
  std::string result = query;
  while (_structures[i] != 0) {
    std::string structure = _structures[i];
    if (query.find(structure) == 0) {
      return query.erase(0, structure.size());
    }
    
    i++;
  }
  return query;
}
