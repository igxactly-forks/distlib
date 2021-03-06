// Copyright (c) 2016 AlertAvert.com. All rights reserved.
// Created by M. Massenzio (marco@alertavert.com) on 3/20/16.


#include <gtest/gtest.h>

#include "../include/MerkleNode.hpp"

using namespace std;

TEST(MerkleNodeTests, CanCreate) {
  std::string s("leaves in spring");

  MD5StringMerkleNode leaf(s);
  ASSERT_TRUE(leaf.validate());
}

TEST(MerkleNodeTests, CanCreateTree) {
  std::string sl[] = {"leaves", "winter", "spring", "flowers"};
  MD5StringMerkleNode *leaves[4];

  for (int i = 0; i < 4; ++i) {
    leaves[i] = new MD5StringMerkleNode(sl[i]);
  }

  MD5StringMerkleNode *a = new MD5StringMerkleNode(leaves[0], leaves[1]);
  MD5StringMerkleNode *b = new MD5StringMerkleNode(leaves[2], leaves[3]);
  MD5StringMerkleNode root(a, b);

  EXPECT_TRUE(root.hasChildren());
  EXPECT_TRUE(b->left() != nullptr);
  EXPECT_TRUE(leaves[3]->right() == nullptr);
  EXPECT_TRUE(root.validate());
}

TEST(MerkleNodeTests, CannotForgeTree) {
  std::string sl[]{"spring", "winter", "summer", "fall"};
  MD5StringMerkleNode *leaves[4];

  for (int i = 0; i < 4; ++i) {
    leaves[i] = new MD5StringMerkleNode(sl[i]);
  }

  MD5StringMerkleNode *a = new MD5StringMerkleNode(leaves[0], leaves[1]);
  MD5StringMerkleNode *b = new MD5StringMerkleNode(leaves[2], leaves[3]);
  MD5StringMerkleNode root(a, b);

  EXPECT_TRUE(root.hasChildren());
  EXPECT_FALSE(b->left() == nullptr);
  EXPECT_TRUE(root.validate());

  MD5StringMerkleNode *fake = new MD5StringMerkleNode("autumn");
  b->setNode(fake, MD5StringMerkleNode::RIGHT);
  EXPECT_FALSE(root.validate());
}

TEST(MerkleNodeTests, hasChildren) {
  MD5StringMerkleNode *a = new MD5StringMerkleNode("test");
  EXPECT_FALSE(a->hasChildren());

  MD5StringMerkleNode *b = new MD5StringMerkleNode("another node");
  MD5StringMerkleNode parent(a, b);
  EXPECT_TRUE(parent.hasChildren());
}

TEST(MerkleNodeTests, Equals) {
  MD5StringMerkleNode a("test");
  MD5StringMerkleNode b("test");

  // First, must be equal to itself.
  EXPECT_EQ(a, a);

  // Then equal to an identically constructed other.
  EXPECT_EQ(a, b);

  // Then, non-equal to a different one.
  MD5StringMerkleNode c("not a test");
  EXPECT_NE(a, c);
}


TEST(MerkleNodeTests, Build) {
  std::string sl[]{"one", "two", "three", "four"};

  std::list<std::string> nodes;
  for (int i = 0; i < 4; ++i) {
    nodes.push_back(sl[i]);
  }

  const MD5StringMerkleNode *root = build<std::string, MD5StringMerkleNode>(nodes);
  ASSERT_NE(nullptr, root);
  EXPECT_TRUE(root->validate());
  delete (root);
}


TEST(MerkleNodeTests, BuildOdd) {

  std::list<std::string> nodes;
  for (int i = 0; i < 33; ++i) {
    nodes.push_back("node #" + to_string(i));
  }

  const MD5StringMerkleNode *root = build<std::string, MD5StringMerkleNode>(nodes);
  ASSERT_NE(nullptr, root);
  EXPECT_TRUE(root->validate());
  delete (root);
}
